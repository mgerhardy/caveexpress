#include "FileSystem.h"
#include "engine/common/Logger.h"
#include "engine/common/System.h"
#include "engine/common/Version.h"
#include <SDL.h>
#ifdef HAVE_SDL_RWHTTP_H
#include <SDL_rwhttp.h>
#endif
#include <iostream>
#include <fstream>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

namespace {
const std::string ROOT = "/$root";
const std::string BASEDIR = "base/" APPNAME;
}

FileSystem::FileSystem () :
		_homeDir(System.getHomeDirectory()), _dataDir(BASEDIR + "/"), _mapsDir("maps/"), _campaignsDir("campaigns/"), _texturesDir(
				"textures/"), _picsDir("pics/"), _soundsDir("sounds/"), _musicDir("music/"), _shaderDir("shaders/"), _languageDir("lang/"), _initialized(
				false)
{
}

FileSystem::~FileSystem ()
{
#ifdef HAVE_SDL_RWHTTP_H
	SDL_RWHttpShutdown();
#endif
}

FileSystem& FileSystem::get ()
{
	static FileSystem instance;
	return instance;
}

void FileSystem::shutdown ()
{
	_initialized = false;
	_dataDir = BASEDIR + "/";
#ifdef HAVE_SDL_RWHTTP_H
	SDL_RWHttpShutdown();
	info(LOG_FILE, "shutdown SDL_rwhttp");
#endif
}

void FileSystem::init (const std::string& protocol, const std::string& protocolPostfix)
{
	if (_initialized) {
		error(LOG_FILE, "called filesystem init at least twice");
		return;
	}
	_initialized = true;
	_protocol = protocol;
	if (_protocol == "file")
		_dataDir = ROOT + "/" + BASEDIR + "/";
	_protocolPostfix = protocolPostfix;
	info(LOG_FILE, "use protocol " + _protocol);
	info(LOG_FILE, "use datadir " + _dataDir);
#ifdef HAVE_SDL_RWHTTP_H
	SDL_RWHttpInit();
	info(LOG_FILE, "init SDL_rwhttp");
#endif
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

	if (!_initialized)
		System.exit("Filesystem is not yet initialized", 1);

	const URI uri("file://" + _homeDir + filename);
	std::string path;
	SDL_RWops *rwops = createRWops(uri, path);
	File file(uri, rwops, path);
	if (!overwrite && file.exists()) {
		info(LOG_FILE, "file already exists: " + uri.print());
		return -1L;
	}
	if (!System.mkdir(_homeDir)) {
		error(LOG_FILE, "could not create directory: " + _homeDir);
		return -1L;
	}
	createDir(file);
	info(LOG_FILE, "writing file " + uri.print());
	return file.write(buf, length);
}

long FileSystem::writeSysFile (const std::string& filename, const unsigned char *buf, size_t length, bool overwrite) const
{
	if (filename.empty())
		return -1L;

	if (!_initialized)
		System.exit("Filesystem is not yet initialized", 1);

	const std::string prot = "file://" + _protocolPostfix;
	const URI uri(prot + replaceSpecialMarkers(getDataDir() + filename, false));
	const std::string path = getPath(uri, false);
	SDL_RWops *rwops = SDL_RWFromFile(path.c_str(), "wb");
	FilePtr file(new File(uri, rwops, path));
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

inline std::string FileSystem::replaceSpecialMarkers (const std::string& path, bool systemRoot) const
{
	const size_t l = ROOT.length();
	if (strncmp(path.c_str(), ROOT.c_str(), l))
		return path;

	if (systemRoot) {
#ifdef PKGDATADIR
		const std::string root = PKGDATADIR;
		return root + path.substr(l);
#else
		return "";
#endif
	}
	if (path.length() > l)
		return System.getCurrentWorkingDir() + path.substr(l + 1);
	// return a relative path like './foo/bar'
	return "." + path.substr(l);
}

SDL_RWops* FileSystem::createRWops (const URI& uri) const
{
	std::string path;
	return createRWops(uri, path);
}

#if defined(EMSCRIPTEN)
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
	}
	if (u.srcDataSize != -1) {
		info(LOG_FILE, "downloaded file " + uri.print());
		return SDL_RWFromMem(u.srcData, u.srcDataSize);
	}

	error(LOG_FILE, "failed to download file " + uri.print());
	return nullptr;
}
#endif

SDL_RWops* FileSystem::createRWops (const URI& uri, std::string& path) const
{
	if (uri.getProtocol() == "file") {
		path = getPath(uri, false);
		SDL_RWops *rwops = SDL_RWFromFile(path.c_str(), "rb");
		if (rwops == nullptr) {
			SDL_ClearError();
			path = getPath(uri, true);
			rwops = SDL_RWFromFile(path.c_str(), "rb");
		}
		return rwops;
	}
	if (uri.getProtocol() == "http" || uri.getProtocol() == "https") {
#if not defined(EMSCRIPTEN)
		const URI uriCached("file://" + getAbsoluteWritePath() + getPath(uri));
		SDL_RWops *cached = createRWops(uriCached, path);
		if (cached != nullptr) {
			info(LOG_FILE, "used cached version: " + uriCached.print());
			return cached;
		}
		SDL_ClearError();
#endif
		info(LOG_FILE, "try to fetch " + uri.print());
#if defined(EMSCRIPTEN)
		SDL_RWops *http = Emscripten_RWFromHttp(uri);
#elif defined(HAVE_SDL_RWHTTP_H)
		SDL_RWops *http = SDL_RWFromHttpSync(uri.print().c_str());
#else
		SDL_RWops *http = nullptr;
#endif
		if (http == nullptr) {
			error(LOG_FILE, "could not download: " + uri.print());
		}
		return http;
	}
	System.exit("No " + uri.getProtocol() + " implemented yet", 1);
	return nullptr;
}

void FileSystem::cache (FilePtr& file, const std::string& subdir) const
{
	if (file->getURI().getProtocol() != "http")
		return;

	const std::string& filename = file->getFileName() + "." + file->getExtension();
	const URI uri("file://" + getAbsoluteWritePath() + subdir + filename);
	std::string path;
	SDL_RWops *rwops = createRWops(uri, path);
	File target(uri, rwops, path);
	if (target.exists())
		return;
	char *buffer;
	int fileLen = file->read((void **) &buffer);
	if (fileLen < 0)
		return;
	System.mkdir(getAbsoluteWritePath());
	createDir(target);
	target.write(reinterpret_cast<const unsigned char*>(buffer), fileLen);
	info(LOG_FILE, "cached " + file->getURI().print());
}

FilePtr FileSystem::getFile (const std::string& filename) const
{
	if (!_initialized)
		System.exit("Filesystem is not yet initialized", 1);

	const std::string prot = _protocol + "://" + _protocolPostfix;

	std::string path;
	if (!getAbsoluteWritePath().empty()) {
		const URI uriLocal(prot + getAbsoluteWritePath() + filename);
		SDL_RWops *rwopsLocal = createRWops(uriLocal, path);
		if (rwopsLocal != nullptr) {
			FilePtr ptr(new File(uriLocal, rwopsLocal, path));
			return ptr;
		}

		SDL_ClearError();
	}
	const URI uri(prot + getDataDir() + filename);
	SDL_RWops *rwops = createRWops(uri, path);
	FilePtr ptr(new File(uri, rwops, path));
	return ptr;
}

FilePtr FileSystem::getFile (const URI& uri) const
{
	std::string path;
	SDL_RWops *rwops = createRWops(uri, path);
	FilePtr ptr(new File(uri, rwops, path));
	return ptr;
}

FilePtr FileSystem::getPic (const std::string& filename) const
{
	const FilePtr& f = getFile(getPicsDir() + filename);
	return f;
}

const std::string FileSystem::getAbsoluteWritePath () const
{
	if (_protocol != "file")
		return "";
	if (_homeDir.empty())
		return "";
	return _homeDir;
}

DirectoryEntries FileSystem::listDirectory (const std::string& basedir, const std::string& subdir)
{
	DirectoryEntries entriesAll;
#ifdef EMSCRIPTEN
	// TODO: move this into files that are generated at build time
	if (basedir == FS.getTexturesDir()) {
		entriesAll.push_back("textures.lua");
		return entriesAll;
	} else if (basedir == FS.getCampaignsDir()) {
		entriesAll.push_back("00-tutorial-campaign.lua");
		entriesAll.push_back("01-ice-campaign.lua");
		entriesAll.push_back("02-rock-campaign.lua");
		entriesAll.push_back("03-second-rock-campaign.lua");
		entriesAll.push_back("04-second-ice-campaign.lua");
		entriesAll.push_back("05-third-ice-campaign.lua");
		entriesAll.push_back("06-wind-campaign.lua");
		return entriesAll;
	}
#endif

	if (!_initialized)
		System.exit("Filesystem is not yet initialized", 1);

	const std::string sysWritePath = replaceSpecialMarkers(getAbsoluteWritePath() + basedir, false);
	if (!sysWritePath.empty())
		entriesAll = System.listDirectory(sysWritePath, subdir);

	const std::string sysDataPath = replaceSpecialMarkers(getDataDir() + basedir, true);
	const DirectoryEntries sysDataEntries = System.listDirectory(sysDataPath, subdir);
	for (DirectoryEntriesIter i = sysDataEntries.begin(); i != sysDataEntries.end(); ++i) {
		entriesAll.push_back(*i);
	}

	const std::string dataDir = replaceSpecialMarkers(getDataDir() + basedir, false);
	const DirectoryEntries entries = System.listDirectory(dataDir, subdir);
	for (DirectoryEntriesIter i = entries.begin(); i != entries.end(); ++i) {
		entriesAll.push_back(*i);
	}
	std::sort(entriesAll.begin(), entriesAll.end());
	entriesAll.erase(std::unique(entriesAll.begin(), entriesAll.end()), entriesAll.end());
	return entriesAll;
}
