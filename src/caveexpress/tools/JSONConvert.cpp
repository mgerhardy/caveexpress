#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <list>
#include <string>
#include <string.h>
#include <Box2D/Common/b2Math.h>
#include <SDL.h>
#include <yajl/yajl_parse.h>
#include "common/Common.h"
#include "common/Log.h"
#include "common/File.h"

const b2Vec2 Vec2Zero(0.0f, 0.0f);
#define VERTICESINDENT "\t\t\t\t"

static int polygonDepth = -1;
static int originDepth = -1;
static int arrayDepth = 0;
static int depth;
static int array;
static int state;
static int vertices = 0;
static float scale = 100.0f;
static float originX = 0.0f;
static float originY = 0.0f;
static int coordinatesParsed = 0;
static std::string onlyName;

static const int PARSEVERTICES = 1 << 1;
static const int PARSENAME = 1 << 2;
static const int WRITEPOLYGON = 1 << 3;
static const int PARSEORIGIN = 1 << 4;
static const int PARSEORIGINX = 1 << 5;
static const int PARSEORIGINY = 1 << 6;
static const int SKIP = 1 << 7;

static void printError (yajl_handle hand, const std::string& str)
{
	unsigned char * error = yajl_get_error(hand, 1, reinterpret_cast<const unsigned char*>(str.c_str()), str.length());
	Log::error(LOG_GAMEIMPL, "%s", reinterpret_cast<const char *>(error));
	yajl_free_error(hand, error);
}

static int convert_null (void * ctx)
{
	return 1;
}

static int convert_boolean (void * ctx, int boolean)
{
	return 1;
}

static int convert_string (void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	const std::string str(reinterpret_cast<const char *>(stringVal), stringLen);
	if (state & PARSENAME) {
		if (!onlyName.empty() && str != onlyName) {
			state |= SKIP;
			return 1;
		} else {
			state &= ~SKIP;
		}
		if (!(state & SKIP)) {
			printf("\t[\"");
			printf("%s", str.c_str());
			printf("\"] = {\n");
		}
		state &= ~PARSENAME;
	}
	return 1;
}

static int convert_number (void * ctx, const char * numberVal, size_t numberLen)
{
	const std::string str(reinterpret_cast<const char *>(numberVal), numberLen);
	if (state & PARSEVERTICES) {
		if (vertices >= 8) {
			vertices = 0;
			if (!(state & SKIP)) {
				printf("\n" VERTICESINDENT);
			}
		}
		const float origin = (++coordinatesParsed % 2) ? originX : originY;
		const float f = (atof(str.c_str()) - origin) * scale;
		vertices++;
		if (vertices != 1) {
			if (!(state & SKIP)) {
				printf(" ");
			}
		}
		if (!(state & SKIP)) {
			printf("%.3f, ", f);
		}
	} else if (state & PARSEORIGINX) {
		originX = atof(str.c_str());
	} else if (state & PARSEORIGINY) {
		originY = atof(str.c_str());
	}
	return 1;
}

static int convert_map_key (void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	const std::string str(reinterpret_cast<const char *>(stringVal), stringLen);
	if (str == "polygons") {
		if (!(state & SKIP)) {
			printf("\t\tpolygons = {\n");
		}
		state |= PARSEVERTICES;
		vertices = 0;
		polygonDepth = depth;
		arrayDepth = array + 1;
	} else if (str == "name") {
		state |= PARSENAME;
	} else if (str == "origin") {
		state |= PARSEORIGIN;
		originDepth = depth;
	}

	if (state & PARSEORIGIN) {
		state &= ~(PARSEORIGINX | PARSEORIGINY);
		if (str == "x") {
			state |= PARSEORIGINX;
		} else if (str == "y") {
			state |= PARSEORIGINY;
		}
	}

	return 1;
}

static int convert_start_map (void * ctx)
{
	depth++;
	return 1;
}

static int convert_end_map (void * ctx)
{
	depth--;
	if (depth < polygonDepth && (state & PARSEVERTICES)) {
		polygonDepth = -1;
		vertices = 0;
		state &= ~PARSEVERTICES;
		if (!(state & SKIP)) {
			printf("\n\t\t},\n");
		}
	} else if (depth == 1) {
		if (!(state & SKIP)) {
			printf("\t},\n");
		}
	}

	if (state & PARSEORIGIN) {
		state &= ~(PARSEORIGINX | PARSEORIGINY | PARSEORIGIN);
	}
	return 1;
}

static int convert_start_array (void * ctx)
{
	array++;
	if (state & WRITEPOLYGON) {
		if (!(state & SKIP)) {
			printf("\t\t\t{\n" VERTICESINDENT "\"\", ");
		}
	} else if (state & PARSEVERTICES) {
		state |= WRITEPOLYGON;
	}
	return 1;
}

static int convert_end_array (void * ctx)
{
	array--;
	if (state & PARSEVERTICES) {
		vertices = 0;
		if (array < arrayDepth) {
			state &= ~PARSEVERTICES;
			state &= ~WRITEPOLYGON;
			if (!(state & SKIP)) {
				printf("\t\t},\n");
			}
		} else {
			if (!(state & SKIP)) {
				printf("\n\t\t\t},\n");
			}
		}
	}
	return 1;
}

static yajl_callbacks callbacks = { convert_null, convert_boolean, nullptr, nullptr, convert_number, convert_string,
		convert_start_map, convert_map_key, convert_end_map, convert_start_array, convert_end_array };

bool readJson (const std::string& str)
{
	yajl_handle hand = yajl_alloc(&callbacks, nullptr, nullptr);
	yajl_status stat = yajl_parse(hand, reinterpret_cast<const unsigned char*>(str.c_str()), str.length());
	if (stat != yajl_status_ok && stat != yajl_status_client_canceled) {
		printError(hand, str);
		return false;
	}

	stat = yajl_complete_parse(hand);
	if (stat != yajl_status_ok && stat != yajl_status_client_canceled) {
		printError(hand, str);
		return false;
	}

	yajl_free(hand);

	return true;
}

static void usage ()
{
	Log::error(LOG_GAMEIMPL, "jsonconverter");
	Log::error(LOG_GAMEIMPL, "  --scale <scale>        - set the vertex scale factor");
	Log::error(LOG_GAMEIMPL, "  --name <name>          - skip all sprites, but the one given in <name>");
	Log::error(LOG_GAMEIMPL, "  --help                 - show this help message");
}

extern "C" int main (int argc, char* argv[])
{
	const std::string path = "contrib/box2deditor/caveexpress.json";
	File file(SDL_RWFromFile(path.c_str(), "rb"), path);
	char *buffer;
	const int fileLen = file.read((void **) &buffer);
	const std::unique_ptr<char[]> p(buffer);
	if (!buffer || fileLen <= 0) {
		Log::error(LOG_GAMEIMPL, "Could not read the json file");
		return EXIT_FAILURE;
	}

	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			if ((!strcmp(argv[i], "-s") || !strcmp(argv[i], "--scale")) && argc >= i + 1) {
				scale = atof(argv[++i]);
				//Log::error(LOG_GAMEIMPL, "use a scale factor of %f", scale);
			} else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
				usage();
				return EXIT_FAILURE;
			} else if ((!strcmp(argv[i], "--name") || !strcmp(argv[i], "-n")) && argc >= i + 1) {
				onlyName = argv[++i];
			} else {
				Log::error(LOG_GAMEIMPL, "unknown command given: %s", argv[i]);
				usage();
				return EXIT_FAILURE;
			}
		}
	}

	const std::string str(buffer, fileLen);
	printf("sprites = {\n");
	const bool ret = readJson(str);
	printf("}\n");
	if (!ret)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
