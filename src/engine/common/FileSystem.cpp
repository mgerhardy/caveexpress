#include "FileSystem.h"
#include "engine/common/Logger.h"
#include "engine/common/System.h"
#include "engine/common/Version.h"
#include <SDL.h>
#include <SDL_platform.h>
#include <iostream>
#include <fstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(__EMSCRIPTEN__) or defined (__NACL__)
// use a pregenerated header with the files
#define DIRLIST_NOT_SUPPORTED 1
#endif

namespace {
const std::string BASEDIR = "base/" APPNAME;
}

FileSystem::FileSystem () :
		_homeDir(System.getHomeDirectory()), _dataDir(BASEDIR + "/"), _mapsDir("maps/"), _campaignsDir("campaigns/"), _texturesDir(
				"textures/"), _picsDir("pics/"), _soundsDir("sounds/"), _musicDir("music/"), _shaderDir("shaders/"), _languageDir("lang/"), _gesturesDir("gestures/")
{
}

FileSystem::~FileSystem ()
{
}

FileSystem& FileSystem::get ()
{
	static FileSystem instance;
	return instance;
}

void FileSystem::shutdown ()
{
}

bool FileSystem::copy (const std::string& src, const std::string& target) const
{
	std::ifstream source(src.c_str(), std::fstream::binary);
	if (source.fail()) {
		error(LOG_FILE, "Opening source file '" + src + "' failed");
		return false;
	}
	std::ofstream dest(target.c_str(), std::fstream::trunc | std::fstream::binary);
	if (dest.fail()) {
		error(LOG_FILE, "Opening dest file '" + target + "' failed");
		return false;
	}
	dest << source.rdbuf();
	source.close();
	dest.close();
	return true;
}

bool FileSystem::deleteFile (const std::string& filename) const
{
	const std::string fullPath = _homeDir + filename;
	return remove(fullPath.c_str()) == 0;
}

long FileSystem::writeFile (const std::string& filename, const unsigned char *buf, size_t length, bool overwrite) const
{
	if (filename.empty())
		return -1L;

	const std::string path = _homeDir + filename;
	SDL_RWops *rwops = createRWops(path, "wb");
	File file(rwops, filename);
	if (!overwrite && file.exists()) {
		info(LOG_FILE, "file already exists: " + path);
		return -1L;
	}
	if (!System.mkdir(_homeDir)) {
		error(LOG_FILE, "could not create directory: " + _homeDir);
		return -1L;
	}
	createDir(file);
	info(LOG_FILE, "writing file " + path);
	return file.write(buf, length);
}

long FileSystem::writeSysFile (const std::string& filename, const unsigned char *buf, size_t length, bool overwrite) const
{
	if (filename.empty())
		return -1L;

	const std::string path = getAbsoluteWritePath() + filename;
	SDL_RWops *rwops = createRWops(path, "wb");
	FilePtr file(new File(rwops, path));
	if (!overwrite && file->exists())
		return -1L;
	const long ret = file->write(buf, length);
	if (ret < 0)
		error(LOG_FILE, "failed to write file " + path);
	else
		info(LOG_FILE, "wrote file " + path + " of size " + string::toString(ret));
	return ret;
}

bool FileSystem::createDir (const File& file) const
{
	const std::string path = file.getPath();
	return System.mkdir(path);
}

bool FileSystem::exists (const std::string& filename) const
{
	const FilePtr& file(getFile(filename));
	return file->exists();
}

SDL_RWops* FileSystem::createRWops (const std::string& path, const std::string& mode) const
{
	SDL_RWops *rwops = SDL_RWFromFile(path.c_str(), mode.c_str());
	SDL_ClearError();
	return rwops;
}

#ifdef __EMSCRIPTEN__
struct Emscripten_UserData {
	void* srcData;
	int srcDataSize;
	bool done;
};

static void Emscripten_OnFailed (void* userData)
{
	Emscripten_UserData* u = (Emscripten_UserData*) userData;
	u->srcDataSize = -1;
	u->done = true;
}

static void Emscripten_OnLoaded (void* userData, void* srcData, int srcDataSize)
{
	Emscripten_UserData* u = (Emscripten_UserData*) userData;
	u->srcData = malloc(srcDataSize);
	u->srcDataSize = srcDataSize;
	memcpy(u->srcData, srcData, srcDataSize);
	u->done = true;
}

static SDL_RWops* Emscripten_RWFromHttp (const URI& uri) {
	Emscripten_UserData u;
	u.done = false;
	u.srcDataSize = -1;
	emscripten_async_wget_data(uri.print().c_str(), (void*)&u, Emscripten_OnLoaded, Emscripten_OnFailed);
	while (!u.done) {
		SDL_Delay(10);
	}
	if (u.srcDataSize != -1) {
		info(LOG_FILE, "downloaded file " + uri.print());
		return SDL_RWFromMem(u.srcData, u.srcDataSize);
	}

	error(LOG_FILE, "failed to download file " + uri.print());
	return nullptr;
}
#endif

FilePtr FileSystem::getFile (const std::string& filename) const
{
	std::string path;
	if (!getAbsoluteWritePath().empty()) {
		path = getAbsoluteWritePath() + getDataDir() + filename;
		SDL_RWops *rwopsLocal = createRWops(path);
		if (rwopsLocal != nullptr) {
			FilePtr ptr(new File(rwopsLocal, path));
			return ptr;
		}
	}
	path = getDataDir() + filename;
	SDL_RWops *rwops = createRWops(path);
	FilePtr ptr(new File(rwops, path));
	return ptr;
}

const std::string FileSystem::getAbsoluteWritePath () const
{
	if (_homeDir.empty())
		return "";
	return _homeDir;
}

DirectoryEntries FileSystem::listDirectory (const std::string& basedir, const std::string& subdir)
{
	DirectoryEntries entriesAll;

#if DIRLIST_NOT_SUPPORTED
#include "dir.h"
#endif

	const std::string sysWritePath = getAbsoluteWritePath() + basedir;
	if (!sysWritePath.empty())
		entriesAll = System.listDirectory(sysWritePath, subdir);

	const std::string dataDir = getDataDir() + basedir;
	const DirectoryEntries entries = System.listDirectory(dataDir, subdir);
	for (DirectoryEntriesIter i = entries.begin(); i != entries.end(); ++i) {
		entriesAll.push_back(*i);
	}
	std::sort(entriesAll.begin(), entriesAll.end());
	entriesAll.erase(std::unique(entriesAll.begin(), entriesAll.end()), entriesAll.end());
	return entriesAll;
}
