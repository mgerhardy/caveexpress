#include <stdlib.h>
#include "engine/common/String.h"
#include "engine/common/FileSystem.h"
#include "engine/common/Logger.h"

extern "C" int main(int argc, char* argv[]) {
	DirectoryEntries entries = FS.listDirectory(FS.getMapsDir());
	for (DirectoryEntriesIter i = entries.begin(); i != entries.end(); ++i) {
		if (!string::endsWith(*i, ".sok"))
			continue;
		info(LOG_SYSTEM, *i);
	}
	return EXIT_SUCCESS;
}
