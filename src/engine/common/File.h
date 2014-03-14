#pragma once

#include "engine/common/URI.h"
#include "engine/common/Compiler.h"
#include "engine/common/String.h"
#include "engine/common/Pointers.h"

struct SDL_RWops;

// define RAWFILE if you don't want rwops to load and store files

class File {
protected:
	URI _uri;
	SDL_RWops* _file;
	FILE* _filePtr;
	bool _useRaw;

	void close ();
	int read (void *buf, size_t size, size_t maxnum);
	long tell () const;
	long seek (long offset, int seekType) const;

public:
#ifdef RAWFILE
	File (const URI& uri, SDL_RWops* file, const std::string& rawPath, bool useRaw = true);
#else
	File (const URI& uri, SDL_RWops* file, const std::string& rawPath, bool useRaw = false);
#endif

	virtual ~File ();

	bool exists () const;
	long length () const;
	std::string getExtension () const;
	std::string getPath () const;
	std::string getFileName () const;

	// returns a new read/write operation structure. It's the callers responsibility to close it
	SDL_RWops* createRWops ();

	long write (const unsigned char *buf, size_t len) const;
	int read (void **buffer);
	int read (void *buffer, int n);
	// get the name of the file - with special placeholder (e.g. $root) included.
	const std::string& getName () const;
	const URI& getURI () const;
};

inline const URI& File::getURI () const
{
	return _uri;
}

typedef SharedPtr<File> FilePtr;
