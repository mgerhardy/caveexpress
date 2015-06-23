#include "File.h"
#include "common/System.h"
#include "common/Config.h"
#include "common/Log.h"
#include <SDL.h>
#ifdef HAVE_SDL_RWHTTP_H
#include <SDL_rwhttp.h>
#endif

File::File (SDL_RWops* file, const std::string& rawPath) :
		_file(file), _rawPath(rawPath)
{
}

File::~File ()
{
	close();
}

const std::string& File::getName () const
{
	return _rawPath;
}

long File::write (const unsigned char *buf, size_t len) const
{
	SDL_RWops *rwops = SDL_RWFromFile(_rawPath.c_str(), "wb");
	if (!rwops) {
		Log::info(LOG_SYSTEM, "failed to get stream for " + _rawPath);
		return -1L;
	}

	int remaining = len;
	while (remaining) {
		const size_t written = SDL_RWwrite(rwops, buf, 1, remaining);
		if (written == 0) {
			Log::info(LOG_SYSTEM, "failed to write to stream " + _rawPath);
			return -1L;
		}

		remaining -= written;
		buf += written;
	}

	SDL_RWclose(rwops);
	getSystem().syncFiles();

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
	return SDL_RWread(_file, buf, size, maxnum);
}

void File::close ()
{
	if (_file != nullptr)
		SDL_RWclose(_file);
}

long File::tell () const
{
	return SDL_RWtell(_file);
}

long File::seek (long offset, int seekType) const
{
	return SDL_RWseek(_file, offset, seekType);
}
