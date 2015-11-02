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

	if (argc != 2) {
		Log::error(LOG_GAMEIMPL, "Usage: %s <sokobansolution>", argv[0]);
		return EXIT_FAILURE;
	}

	const std::string file = argv[1];
	Log::info(LOG_GAMEIMPL, "Convert %s to rle encoded solution", file.c_str());

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
	Log::info(LOG_GAMEIMPL, "original solution: %s", orig.c_str());
	Log::info(LOG_GAMEIMPL, "rle compressed solution: %s", rle.c_str());

	handle->write((const unsigned char*)rle.c_str(), rle.length(), "wb");

	return EXIT_SUCCESS;
}
