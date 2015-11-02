#include <stdlib.h>
#include "common/Log.h"
#include "common/FileSystem.h"
#include "common/Application.h"
#include "common/System.h"
#include "game/GameRegistry.h"
#include "cavepacker/shared/SolutionUtil.h"
#include <SDL.h>
#include <SDL_main.h>

using namespace cavepacker;

extern "C" int main(int argc, char* argv[]) {
	Application& app = Singleton<Application>::getInstance();
	app.setOrganisation("caveproductions");
	app.setName("cavepacker");

	if (argc < 2) {
		Log::error(LOG_GAMEIMPL, "Usage: %s <options> <sokobansolution>", argv[0]);
		Log::info(LOG_GAMEIMPL, " -o --overwrite     - overwrite the original solution file");
		Log::info(LOG_GAMEIMPL, " -v --verbose       - be verbose");
		return EXIT_FAILURE;
	}

	bool overwriteSolution = false;
	bool verbose = false;

	for (int i = 1; i < argc - 1; ++i) {
		const std::string arg = argv[i];
		if (arg == "-o" || arg == "--overwrite")
			overwriteSolution = true;
		else if (arg == "-v" || arg == "--verbose")
			verbose = true;
	}

	const std::string file = argv[argc - 1];
	if (verbose) {
		Log::info(LOG_GAMEIMPL, "Convert %s to rle encoded solution", file.c_str());
	}

	const FilePtr handle = FS.getFile(file);
	if (!handle) {
		Log::error(LOG_GAMEIMPL, "Could not load %s", file.c_str());
		return EXIT_FAILURE;
	}

	char *buffer;
	const int fileLen = handle->read((void **) &buffer);
	std::unique_ptr<char[]> p(buffer);
	if (!buffer || fileLen <= 0) {
		Log::error(LOG_GAMEIMPL, "Solution file '%s' can't get loaded", handle->getName().c_str());
		return EXIT_FAILURE;
	}

	std::string orig(buffer, fileLen);
	const std::string& rle = SolutionUtil::compress(orig);
	const std::string& decompress = SolutionUtil::decompress(rle);
	if (orig != decompress) {
		Log::error(LOG_GAMEIMPL, "Failed to compress the solution:");
		Log::error(LOG_GAMEIMPL, "orig: %s", orig.c_str());
		Log::error(LOG_GAMEIMPL, "rle: %s", rle.c_str());
		Log::error(LOG_GAMEIMPL, "back:  %s", decompress.c_str());
		overwriteSolution = false;
	}
	if (verbose) {
		Log::info(LOG_GAMEIMPL, "original solution: %s", orig.c_str());
		Log::info(LOG_GAMEIMPL, "rle compressed solution: %s", rle.c_str());
	}

	if (overwriteSolution) {
		if (verbose) {
			Log::info(LOG_GAMEIMPL, "overwrite solution");
		}
		handle->write((const unsigned char*)rle.c_str(), rle.length(), "wb");
	}

	return EXIT_SUCCESS;
}
