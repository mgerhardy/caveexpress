#pragma once

#include "common/File.h"
#include "common/ports/ISystem.h"
#include "common/NonCopyable.h"
#include <unordered_map>

struct SDL_RWops;

class FileSystem: public NonCopyable {
private:
	std::unordered_map<std::string, std::string> _schemes;
	std::string _homeDir;
	std::string _dataDir;
	const std::string _mapsDir;
	const std::string _campaignsDir;
	const std::string _texturesDir;
	const std::string _picsDir;
	const std::string _soundsDir;
	const std::string _musicDir;
	const std::string _shaderDir;
	const std::string _languageDir;
	const std::string _gesturesDir;

	std::string getDirForURLType (const std::string& type) const;

	FileSystem ();
public:
	virtual ~FileSystem ();

	static FileSystem& get ();

	SDL_RWops* createRWops (const std::string& file, const std::string& mode = "rb") const;
	void shutdown ();
	void registerURL(const std::string& type, const std::string& dir);
	// writes a file to the users home directory
	long writeFile (const std::string& filename, const unsigned char *buf, size_t length, bool overwrite = false) const;
	// writes a file into the game system wide data directory
	long writeSysFile (const std::string& filename, const unsigned char *buf, size_t length, bool overwrite) const;
	bool exists (const std::string& filename) const;
	bool createDir (const File& file) const;
	bool deleteFile (const std::string& filename) const;
	bool copy (const std::string& src, const std::string& target) const;
	FilePtr getFile (const std::string& filename) const;
	FilePtr getFileFromURL (const std::string& filename) const;
	const std::string& getDataDir () const;
	const std::string& getMapsDir () const;
	const std::string& getCampaignsDir () const;
	const std::string& getTexturesDir () const;
	const std::string& getPicsDir () const;
	const std::string& getSoundsDir () const;
	const std::string& getMusicDir () const;
	const std::string& getLanguageDir () const;
	const std::string& getGesturesDir () const;
	const std::string& getShaderDir () const;
	const std::string getAbsoluteWritePath () const;

	DirectoryEntries listDirectory (const std::string& basedir, const std::string& subdir = "") const;

	bool hasExtension (const std::string& filename, const std::string& extension) const;
};

inline bool FileSystem::hasExtension (const std::string& filename, const std::string& extension) const
{
	const std::string extStr = "." + extension;
	const std::size_t strLength = filename.length();
	const std::size_t endLength = extStr.length();
	if (strLength >= endLength) {
		const std::size_t index = strLength - endLength;
		return filename.compare(index, endLength, extStr) == 0;
	}
	return false;
}

inline const std::string& FileSystem::getDataDir () const
{
	return _dataDir;
}

inline const std::string& FileSystem::getMapsDir () const
{
	return _mapsDir;
}

inline const std::string& FileSystem::getCampaignsDir () const
{
	return _campaignsDir;
}

inline const std::string& FileSystem::getTexturesDir () const
{
	return _texturesDir;
}

inline const std::string& FileSystem::getPicsDir () const
{
	return _picsDir;
}

inline const std::string& FileSystem::getSoundsDir () const
{
	return _soundsDir;
}

inline const std::string& FileSystem::getMusicDir () const
{
	return _musicDir;
}

inline const std::string& FileSystem::getLanguageDir () const
{
	return _languageDir;
}

inline const std::string& FileSystem::getGesturesDir () const
{
	return _gesturesDir;
}

inline const std::string& FileSystem::getShaderDir () const
{
	return _shaderDir;
}

#define FS FileSystem::get()
