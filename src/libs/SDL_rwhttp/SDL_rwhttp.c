#ifndef SKIP_CONFIG_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#endif

#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
#include <SDL_net.h>
#endif
#include "SDL_rwhttp.h"

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

static const char *userAgent;
static int connectTimeout;
static int timeout;
static int fetchLimit;
static const char *contentLength = "Content-Length: ";
static const size_t strLength = 16;

int SDL_RWHttpShutdown (void)
{
#ifdef HAVE_CURL
	curl_global_cleanup();
#else
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
	SDLNet_Quit();
#endif
#endif
	return 0;
}

int SDL_RWHttpInit (void)
{
	const char *hint;

	userAgent = SDL_GetHint(SDL_RWHTTP_HINT_USER_AGENT);
	if (!userAgent)
		userAgent = "SDL_rwhttp/" STRINGIFY(SDL_RWHTTP_MAJOR_VERSION) "." STRINGIFY(SDL_RWHTTP_MINOR_VERSION);

	hint = SDL_GetHint(SDL_RWHTTP_HINT_CONNECTTIMEOUT);
	if (hint)
		connectTimeout = atoi(hint);
	else
		connectTimeout = 3;

	hint = SDL_GetHint(SDL_RWHTTP_HINT_TIMEOUT);
	if (hint)
		timeout = atoi(hint);
	else
		timeout = 3;

	hint = SDL_GetHint(SDL_RWHTTP_HINT_FETCHLIMIT);
	if (hint)
		fetchLimit = atoi(hint);
	else
		fetchLimit = 1024 * 1024 * 5;

#ifdef HAVE_CURL
	{
		const CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
		if (result == CURLE_OK) {
			return 0;
		}
		SDL_SetError(curl_easy_strerror(result));
	}
#else
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
	{
		if (SDLNet_Init() >= 0) {
			/* The sdl error is already set */
			return 0;
		}
		return -1;
	}
#endif
#endif

	SDL_SetError("Required library is missing");
	return -1;
}

typedef struct {
	char *uri;
	char *data;
	size_t size;
	size_t expectedSize;
	void *userData;
	Uint8 *base;
	Uint8 *here;
	Uint8 *stop;
} http_data_t;

static int http_close (SDL_RWops * context)
{
	int status = 0;
	if (!context) {
		return status;
	}

	if (context->hidden.unknown.data1) {
		http_data_t *data = (http_data_t*) context->hidden.unknown.data1;
#ifdef HAVE_CURL
		CURL *curl = (CURL*)data->userData;
		curl_easy_cleanup(curl);
#endif
		SDL_free(data->data);
		SDL_free(data);
	}
	SDL_FreeRW(context);
	return status;
}

static Sint64 http_size (SDL_RWops * context)
{
	const http_data_t *data = (const http_data_t*) context->hidden.unknown.data1;
	return (Sint64)(data->stop - data->base);
}

static Sint64 http_seek (SDL_RWops * context, Sint64 offset, int whence)
{
	http_data_t *data = (http_data_t*) context->hidden.unknown.data1;
	Uint8 *newpos;

	switch (whence) {
	case RW_SEEK_SET:
		newpos = data->base + offset;
		break;
	case RW_SEEK_CUR:
		newpos = data->here + offset;
		break;
	case RW_SEEK_END:
		newpos = data->stop + offset;
		break;
	default:
		return SDL_SetError("Unknown value for 'whence'");
	}
	if (newpos < data->base) {
		newpos = data->base;
	}
	if (newpos > data->stop) {
		newpos = data->stop;
	}
	data->here = newpos;
	return (Sint64)(data->here - data->base);
}

static size_t http_read (SDL_RWops * context, void *ptr, size_t size, size_t maxnum)
{
	http_data_t *data = (http_data_t*) context->hidden.unknown.data1;
	size_t total_bytes;
	size_t mem_available;

	total_bytes = (maxnum * size);
	if (maxnum <= 0 || size <= 0 || total_bytes / maxnum != (size_t) size) {
		return 0;
	}

	mem_available = data->stop - data->here;
	if (total_bytes > mem_available) {
		total_bytes = mem_available;
	}

	SDL_memcpy(ptr, data->here, total_bytes);
	data->here += total_bytes;

	return total_bytes / size;
}

static size_t http_writeconst (SDL_RWops * context, const void *ptr, size_t size, size_t num)
{
	SDL_SetError("Can't write to read-only memory");
	return 0;
}

static size_t SDL_RWHttpHeader (void *headerData, size_t size, size_t nmemb, void *userData)
{
	const char *header = (const char *)headerData;
	const size_t bytes = size * nmemb;

	if (bytes <= strLength) {
		return bytes;
	}

	if (!SDL_strncasecmp(header, contentLength, strLength)) {
		http_data_t *httpData = (http_data_t *) userData;
		httpData->expectedSize = SDL_strtoul(header + strLength, NULL, 10);
		if (httpData->expectedSize == 0) {
			SDL_SetError("invalid content length given: %i", (int)httpData->expectedSize);
			return 0;
		}
		if (httpData->expectedSize > fetchLimit) {
			SDL_SetError("content length exceeded the hardcoded limit of %i (%i)", fetchLimit, (int)httpData->expectedSize);
			return 0;
		}
		httpData->data = SDL_malloc(httpData->expectedSize);
		if (httpData->data == NULL) {
			SDL_SetError("not enough memory (malloc returned NULL)");
			return 0;
		}
	}

	return bytes;
}

static size_t SDL_RWHttpWrite (void *streamData, size_t size, size_t nmemb, void *userData)
{
	const size_t realsize = size * nmemb;
	http_data_t *httpData = (http_data_t *) userData;

	if (httpData->expectedSize == 0) {
		const int newSize = httpData->size + realsize;
		if (newSize > fetchLimit) {
			SDL_SetError("file exceeded the hardcoded limit of %i (%i)", fetchLimit, (int)newSize);
			return 0;
		}
		httpData->data = SDL_realloc(httpData->data, newSize);
		if (httpData->data == NULL) {
			SDL_SetError("not enough memory (realloc returned NULL)");
			return 0;
		}
	} else if (httpData->size + realsize > httpData->expectedSize) {
		SDL_SetError("illegal Content-Length - buffer overflow");
		return 0;
	}

	SDL_memcpy(&(httpData->data[httpData->size]), streamData, realsize);
	httpData->size += realsize;

	return realsize;
}

#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
#define READ_SIZE 512
static int SDL_RWHttpSDLNetDownload (http_data_t *httpData, TCPsocket socket, const char *uri, int port)
{
	size_t bufSize = 512;
	Uint8 *buf, *bufPtr;
	const int length = 80 + strlen(uri) + strlen(userAgent);
	char *request = SDL_malloc(length);
	char *host;
	SDL_bool headerParsed = SDL_FALSE;
	char *path;

	if (request == NULL) {
		SDL_SetError("not enough memory (malloc returned NULL)");
		return -1;
	}

	host = SDL_strdup(uri);
	path = SDL_strchr(host, '/');
	if (path) {
		path[0] = '\0';
	}
	SDL_snprintf(request, length - 1, "GET /%s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\nUser-Agent: %s\r\n\r\n", path ? path + 1 : "", host, userAgent);
	SDL_free(host);

	if (SDLNet_TCP_Send(socket, request, strlen(request)) < strlen(request)) {
		SDL_SetError("sending the request '%s' failed", request);
		SDL_free(request);
		return -1;
	}
	SDL_free(request);

	bufPtr = buf = SDL_malloc(bufSize);
	for (;;) {
		const int read = SDLNet_TCP_Recv(socket, (void*) bufPtr, READ_SIZE - 1);
		if (read <= -1) {
			SDL_free(buf);
			return -1;
		}
		if (read == 0) {
			break;
		}
		if (headerParsed) {
			SDL_RWHttpWrite(bufPtr, 1, read, httpData);
		} else {
			char *headerEnd;
			bufPtr[read] = '\0';
			headerEnd = SDL_strstr((char *)bufPtr, "\r\n\r\n");
			if (headerEnd != NULL) {
				const ptrdiff_t bodySize = read - ((Uint8*)headerEnd + 4 - bufPtr);
				char *headerLine;
				char *headerBegin;
				if (bodySize > 0) {
					SDL_RWHttpWrite(headerEnd + 4, 1, bodySize, httpData);
				}
				headerParsed = SDL_TRUE;
				headerLine = SDL_strdup((const char *)buf);
				headerBegin = headerLine;
				for (;;) {
					char *newline = SDL_strstr(headerLine, "\r\n");
					if (newline >= headerEnd) {
						break;
					} else if (newline == NULL) {
						SDL_free(buf);
						SDL_free(headerBegin);
						SDL_SetError("invalid header entry");
						return -1;
					} else {
						newline[0] = '\0';
					}

					if (SDL_RWHttpHeader(headerLine, 1, strlen(headerLine), httpData) == 0) {
						SDL_free(buf);
						SDL_free(headerBegin);
						return -1;
					}
					headerLine = newline + 2;
					if (headerLine >= headerEnd) {
						break;
					}
				}
				SDL_free(headerBegin);
			} else {
				const ptrdiff_t pos = bufPtr - buf + read;
				bufSize += READ_SIZE;
				if (bufSize > fetchLimit) {
					SDL_free(buf);
					SDL_SetError("header exceeded the hardcoded limit of %i (%i)", fetchLimit, bufSize);
					return -1;
				}
				buf = SDL_realloc(buf, bufSize);
				bufPtr = &buf[pos];
			}
		}
	}
	SDL_free(buf);

	return 0;
}
#endif

static SDL_RWops* SDL_RWHttpCreate (http_data_t *httpData)
{
	SDL_RWops *rwops = SDL_AllocRW();
	if (!rwops)
		return NULL;

	rwops->size = http_size;
	rwops->seek = http_seek;
	rwops->read = http_read;
	rwops->write = http_writeconst;
	rwops->close = http_close;

	httpData->base = (Uint8*) httpData->data;
	httpData->here = httpData->base;
	httpData->stop = httpData->base + httpData->size;

	rwops->hidden.unknown.data1 = httpData;
	rwops->type = SDL_RWOPS_HTTP;
	return rwops;
}

SDL_RWops* SDL_RWFromHttpSync (const char *uri)
{
	SDL_RWops *rwops;
	http_data_t *httpData;
#ifdef HAVE_CURL
	CURL* curlHandle;
	CURLcode result;
#else
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
	IPaddress ip;
	int port = 80;
	TCPsocket socket;
	char *host;
#endif
#endif

	if (!uri || uri[0] == '\0') {
		SDL_SetError("No uri given");
		return NULL;
	}

	httpData = SDL_malloc(sizeof(*httpData));
	SDL_zerop(httpData);

#ifdef HAVE_CURL
	curlHandle = curl_easy_init();
	curl_easy_setopt(curlHandle, CURLOPT_URL, uri);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, SDL_RWHttpWrite);
	curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, SDL_RWHttpHeader);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, (void * )httpData);
	curl_easy_setopt(curlHandle, CURLOPT_HEADERDATA, (void * )httpData);
	curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, userAgent);
	curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curlHandle, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curlHandle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, connectTimeout);
	curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, timeout);

	httpData->userData = curlHandle;

	result = curl_easy_perform(curlHandle);
	if (result != CURLE_OK) {
		SDL_SetError(curl_easy_strerror(result));
		SDL_free(httpData->data);
		SDL_free(httpData);
		return NULL;
	}
#else
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
	if (SDL_strstr(uri, "://") != NULL) {
		uri = SDL_strstr(uri, "://") + 3;
	}
	if (SDL_strchr(uri, ':')) {
		port = atoi(SDL_strchr(uri, ':') + 1);
	}

	host = SDL_strdup(uri);
	if (SDL_strchr(host, ':')) {
		SDL_strchr(host, ':')[0] = '\0';
	}
	if (SDL_strchr(host, '/')) {
		SDL_strchr(host, '/')[0] = '\0';
	}

	if (SDLNet_ResolveHost(&ip, host, port) < 0) {
		SDL_free(host);
		SDL_free(httpData);
		/* sdl error is already set */
		return NULL;
	}
	SDL_free(host);

	if (!(socket = SDLNet_TCP_Open(&ip))) {
		SDL_free(httpData);
		return NULL;
	}

	if (SDL_RWHttpSDLNetDownload(httpData, socket, uri, port) == -1) {
		SDLNet_TCP_Close(socket);
		SDL_free(httpData->data);
		SDL_free(httpData);
		return NULL;
	}

	SDLNet_TCP_Close(socket);
#endif
#endif

	rwops = SDL_RWHttpCreate(httpData);
	if (!rwops) {
		SDL_SetError("Could not fetch the data from %s", uri);
	}
	return rwops;
}

SDL_RWops* SDL_RWFromHttpAsync (const char *uri)
{
	SDL_SetError("Not yet supported");
	return NULL;
}
