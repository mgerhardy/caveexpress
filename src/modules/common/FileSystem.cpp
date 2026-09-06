#include "FileSystem.h"
#include "common/Log.h"
#include "common/System.h"
#include "common/Application.h"
#include "../game/GameRegistry.h"
#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_platform.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

FileSystem::FileSystem () :
		_homeDir(System.getHomeDirectory()), _dataDir("base/" + Singleton<Application>::getInstance().getName() + "/"), _mapsDir("maps/"), _campaignsDir("campaigns/"), _texturesDir(
				"textures/"), _picsDir("pics/"), _soundsDir("sounds/"), _musicDir("music/"), _shaderDir("shaders/"), _languageDir("lang/"), _gesturesDir("gestures/")
{
#ifdef PKGDATADIR
	_dataDir = PKGDATADIR;
	if (!string::endsWith(_dataDir, "/")) {
		_dataDir += "/";
	}
	_dataDir += Singleton<Application>::getInstance().getName() + "/";
#else
	const char *snap = SDL_getenv("SNAP");
	if (snap != nullptr && snap[0] != '\0') {
		_dataDir = snap;
		if (!string::endsWith(_dataDir, "/")) {
			_dataDir += "/";
		}
	}
#endif
	registerURL("maps", getMapsDir());
	registerURL("textures", getTexturesDir());
	registerURL("pics", getPicsDir());
	registerURL("music", getMusicDir());
	registerURL("shaders", getShaderDir());
	registerURL("languages", getLanguageDir());
	registerURL("sounds", getSoundsDir());
	registerURL("campaigns", getCampaignsDir());
	registerURL("gestures", getGesturesDir());
	registerURL("home", _homeDir);
}

namespace {
int SDLCALL closeTrackedRWops (SDL_RWops *rwops)
{
	return FileSystem::get().onTrackedRWopsClose(rwops);
}
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

bool FileSystem::copy (const std::string& src, const std::string& target) const {
	if (!beginOpen(src))
		return false;
	FILE *f = fopen(src.c_str(), "rb");
	if (!f) {
		endOpen(src);
		Log::error(LOG_COMMON, "Opening source file '%s' failed", src.c_str());
		return false;
	}

	if (!beginOpen(target)) {
		fclose(f);
		endOpen(src);
		return false;
	}
	FILE* ft = fopen(target.c_str(), "wb");
	if (!ft) {
		fclose(f);
		endOpen(src);
		endOpen(target);
		return false;
	}
	fseek(f, 0, SEEK_END);
	const size_t len = static_cast<size_t>(ftell(f));
	fseek(f, 0, SEEK_SET);

	unsigned char *const buf = (unsigned char *) malloc(len);
	if (fread(buf, 1, len, f) != len) {
		fclose(f);
		fclose(ft);
		endOpen(src);
		endOpen(target);
		free(buf);
		return false;
	}
	fclose(f);
	endOpen(src);

	if (fwrite(buf, 1, len, ft) != len) {
		Log::error(LOG_COMMON, "Opening dest file '%s' failed", target.c_str());
		free(buf);
		fclose(ft);
		endOpen(target);
		return false;
	}

	free(buf);
	fclose(ft);
	endOpen(target);
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
	File file(rwops, path);
	if (!overwrite && file.exists()) {
		Log::info(LOG_COMMON, "file already exists: %s", path.c_str());
		return -1L;
	}
	if (!System.mkdir(_homeDir)) {
		Log::error(LOG_COMMON, "could not create directory: %s",  _homeDir.c_str());
		return -1L;
	}
	createDir(file);
	Log::info(LOG_COMMON, "writing file %s", path.c_str());
	return file.write(buf, length);
}

long FileSystem::writeSysFile (const std::string& filename, const unsigned char *buf, size_t length, bool overwrite) const
{
	if (filename.empty())
		return -1L;

	SDL_RWops *rwops = createRWops(filename, "wb");
	FilePtr file(new File(rwops, filename));
	if (!overwrite && file->exists())
		return -1L;
	const long ret = file->write(buf, length);
	if (ret < 0)
		Log::error(LOG_COMMON, "failed to write file %s", filename.c_str());
	else
		Log::info(LOG_COMMON, "wrote file %s of size %li", filename.c_str(), ret);
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
	if (!beginOpen(path))
		return nullptr;

	SDL_RWops *rwops = SDL_RWFromFile(path.c_str(), mode.c_str());
	if (rwops == nullptr) {
		endOpen(path);
		SDL_ClearError();
		return nullptr;
	}

	_openedRWops[rwops] = normalizeOpenPath(path);
	_origRWopsClose[rwops] = reinterpret_cast<void*>(rwops->close);
	rwops->close = closeTrackedRWops;
	return rwops;
}

std::string FileSystem::normalizeOpenPath (const std::string& path) const
{
	std::string normalized = path;
	for (char& c : normalized) {
		if (c == '\\')
			c = '/';
	}
	return normalized;
}

bool FileSystem::beginOpen (const std::string& path) const
{
	const std::string key = normalizeOpenPath(path);
	if (_openedFiles.find(key) != _openedFiles.end()) {
		Log::error(LOG_COMMON, "file already open: %s", key.c_str());
		for (const std::string& open : _openedFiles) {
			Log::error(LOG_COMMON, "  still open: %s", open.c_str());
		}
		SDL_assert_always(!"duplicate file open");
		return false;
	}
	_openedFiles.insert(key);
	return true;
}

void FileSystem::endOpen (const std::string& path) const
{
	_openedFiles.erase(normalizeOpenPath(path));
}

int FileSystem::onTrackedRWopsClose (SDL_RWops *rwops)
{
	if (rwops == nullptr)
		return 0;

	auto pathIter = _openedRWops.find(rwops);
	if (pathIter != _openedRWops.end()) {
		endOpen(pathIter->second);
		_openedRWops.erase(pathIter);
	}

	using RWopsCloseFn = int (SDLCALL *)(SDL_RWops *);
	RWopsCloseFn orig = nullptr;
	auto origIter = _origRWopsClose.find(rwops);
	if (origIter != _origRWopsClose.end()) {
		orig = reinterpret_cast<RWopsCloseFn>(origIter->second);
		_origRWopsClose.erase(origIter);
	}

	if (orig == nullptr)
		return 0;
	rwops->close = orig;
	return orig(rwops);
}

FilePtr FileSystem::getFile (const std::string& filename) const
{
	if (!getAbsoluteWritePath().empty()) {
		const std::string path = getAbsoluteWritePath() + getDataDir() + filename;
		SDL_RWops *rwopsLocal = createRWops(path);
		if (rwopsLocal != nullptr) {
			FilePtr ptr(new File(rwopsLocal, path));
			return ptr;
		}
	}
	const std::string path = getDataDir() + filename;
	SDL_RWops *rwops = createRWops(path);
	FilePtr ptr(new File(rwops, path));
	return ptr;
}

std::string FileSystem::getDirForURLType (const std::string& type) const
{
	auto i = _schemes.find(type);
	if (i == _schemes.end())
		return "";
	return i->second;
}

void FileSystem::registerURL(const std::string& type, const std::string& dir)
{
	_schemes[type] = dir;
}

FilePtr FileSystem::getFileFromURL (const std::string& filename) const
{
	std::string::size_type pos = filename.find_first_of("://", 0);
	if (pos == std::string::npos) {
		System.exit("Not a valid url: " + filename, 1);
	}

	const std::string& type = filename.substr(0, pos);
	const std::string& dir = getDirForURLType(type);
	if (dir.empty()) {
		System.exit("No type registered for " + type, 1);
	}
	const std::string& name = filename.substr(pos + 3, filename.length());
	return getFile(dir + name);
}

const std::string FileSystem::getAbsoluteWritePath () const
{
	return _homeDir;
}

DirectoryEntries FileSystem::listDirectory (const std::string& basedir, const std::string& subdir) const
{
	DirectoryEntries entriesAll;
	const std::string sysWritePath = getAbsoluteWritePath() + basedir;
	if (!sysWritePath.empty())
		entriesAll = System.listDirectory(sysWritePath, subdir);

	const std::string dataDir = getDataDir() + basedir;
	const DirectoryEntries entries = System.listDirectory(dataDir, subdir);
	for (const std::string& entry : entries) {
		entriesAll.push_back(entry);
	}
	std::sort(entriesAll.begin(), entriesAll.end());
	entriesAll.erase(std::unique(entriesAll.begin(), entriesAll.end()), entriesAll.end());
	return entriesAll;
}
