#pragma once

#include "common/URI.h"
#include "common/Compiler.h"
#include "common/Pointers.h"
#include <string>

struct SDL_RWops;

// define RAWFILE if you don't want rwops to load and store files

class File {
protected:
	SDL_RWops* _file;
	const std::string _rawPath;

	void close ();
	int read (void *buf, size_t size, size_t maxnum);
	long tell () const;
	long seek (long offset, int seekType) const;

public:
	File (SDL_RWops* file, const std::string& rawPath);

	virtual ~File ();

	bool exists () const;
	long length () const;
	std::string getExtension () const;
	std::string getPath () const;
	std::string getFileName () const;

	long write (const unsigned char *buf, size_t len) const;
	int read (void **buffer);
	int read (void *buffer, int n);
	// get the name of the file - with special placeholder (e.g. $root) included.
	const std::string& getName () const;
};

typedef SharedPtr<File> FilePtr;
