#include "File.h"
#include "engine/common/System.h"
#include "engine/common/Config.h"
#include "engine/common/Logger.h"
#include <SDL.h>
#ifdef HAVE_SDL_RWHTTP_H
#include <SDL_rwhttp.h>
#endif

File::File (const URI& uri, SDL_RWops* file, const std::string& rawPath, bool useRaw) :
		_uri(uri), _file(file), _useRaw(useRaw), _filePtr(nullptr)
{
	if (_useRaw) {
		_filePtr = ::fopen(rawPath.c_str(), "rb");
		if (_filePtr == nullptr)
			error(LOG_SYSTEM, "could not open " + rawPath);
		else if (fileno(_filePtr) == -1)
			error(LOG_SYSTEM, "invalid stream opened " + rawPath);
	}
}

File::~File ()
{
	close();
}

const std::string& File::getName () const
{
	return _uri.getPath();
}

long File::write (const unsigned char *buf, size_t len) const
{
	if (_uri.getProtocol() != "file") {
		return -2000L;
	}

	if (_useRaw) {
		FILE* fileHandle = ::fopen(_uri.getPath().c_str(), "wb");
		if (fileHandle == nullptr) {
			info(LOG_SYSTEM, "failed to get stream for " + _uri.print());
			return -1L;
		}

		int remaining = len;
		while (remaining) {
			const size_t written = ::fwrite(buf, 1, remaining, fileHandle);
			if (written == 0) {
				info(LOG_SYSTEM, "failed to write to stream " + _uri.print());
				return -1L;
			}

			remaining -= written;
			buf += written;
		}

		::fclose(fileHandle);

		return len;
	}

	const char *path = _uri.getPath().c_str();
	SDL_RWops *rwops = SDL_RWFromFile(path, "wb");
	if (!rwops) {
		info(LOG_SYSTEM, "failed to get stream for " + _uri.print());
		return -1L;
	}

	int remaining = len;
	while (remaining) {
		const size_t written = SDL_RWwrite(rwops, buf, 1, remaining);
		if (written == 0) {
			info(LOG_SYSTEM, "failed to write to stream " + _uri.print());
			return -1L;
		}

		remaining -= written;
		buf += written;
	}

	SDL_RWclose(rwops);

	return len;
}

std::string File::getPath () const
{
	const std::string& name = getName();
	const size_t pos = name.rfind("/");
	if (pos == std::string::npos)
		return "";
	return name.substr(0, pos);
}

std::string File::getFileName () const
{
	std::string name = getName();
	const size_t pathPos = name.rfind("/");
	if (pathPos != std::string::npos) {
		name = name.substr(pathPos + 1);
	}
	const size_t extPos = name.rfind(".");
	if (extPos != std::string::npos) {
		name = name.substr(0, extPos);
	}
	return name;
}

std::string File::getExtension () const
{
	const char *ext = ::strrchr(getName().c_str(), '.');
	if (ext == nullptr)
		return "";
	++ext;
	return std::string(ext);
}

long File::length () const
{
	if (!exists()) {
		return -1;
	}

	const long pos = tell();
	seek(0, RW_SEEK_END);
	const long end = tell();
	seek(pos, RW_SEEK_SET);
	return end;
}

bool File::exists () const
{
	return _file != nullptr;
}

int File::read (void **buffer)
{
	*buffer = nullptr;
	const long len = length();
	if (len <= 0)
		return len;
	*buffer = new char[len];
	return read(*buffer, len);
}

int File::read (void *buffer, int n)
{
	const size_t blockSize = 0x10000;
	unsigned char *buf;
	size_t remaining, len;

	len = remaining = n;
	buf = (unsigned char *) buffer;

	seek(0, RW_SEEK_SET);

	while (remaining) {
		size_t block = remaining;
		if (block > blockSize)
			block = blockSize;
		const int readAmount = read(buf, 1, block);

		/* end of file reached */
		if (readAmount == 0)
			return (len - remaining + readAmount);

		else if (readAmount == -1)
			return -1;

		/* do some progress bar thing here... */
		remaining -= readAmount;
		buf += readAmount;
	}
	return len;
}

int File::read (void *buf, size_t size, size_t maxnum)
{
	if (_useRaw)
		return ::fread(buf, size, maxnum, _filePtr);
	return SDL_RWread(_file, buf, size, maxnum);
}

void File::close ()
{
	if (_file != nullptr)
		SDL_RWclose(_file);
	if (_useRaw) {
		if (fileno(_filePtr) != -1) {
			::fclose(_filePtr);
		}
		_filePtr = nullptr;
	}
}

long File::tell () const
{
	if (_useRaw)
		return ::ftell(_filePtr);
	return SDL_RWtell(_file);
}

long File::seek (long offset, int seekType) const
{
	if (_useRaw) {
		switch (seekType) {
		case RW_SEEK_END:
			return ::fseek(_filePtr, offset, SEEK_END);
		case RW_SEEK_SET:
			return ::fseek(_filePtr, offset, SEEK_SET);
		case RW_SEEK_CUR:
			return ::fseek(_filePtr, offset, SEEK_CUR);
		default:
			return -1L;
		}
	}
	return SDL_RWseek(_file, offset, seekType);
}
