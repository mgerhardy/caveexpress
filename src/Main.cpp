#include "mainloop/SDLMainLoop.h"
#include "common/ConfigManager.h"
#include "common/Log.h"
#include "common/Application.h"
#include "common/System.h"
#include "game/GameRegistry.h"
#include <SDL.h>
#include <SDL_main.h>
#include "game.h"

extern "C" int main (int argc, char *argv[])
{
	Application& app = Singleton<Application>::getInstance();
	const GamePtr& game = Singleton<GameRegistry>::getInstance().getGame();
	app.setOrganisation("caveproductions");
#if defined(__IPHONEOS__) || defined(__ANDROID__)
	app.setPackageName(GUI_IDENTIFIER);
#else
	app.setPackageName(APPNAME);
#endif
	app.setName(game->getName());
	app.setVersion(VERSION);

	getSystem().init();
	const std::string workingDir = getSystem().getCurrentWorkingDir();
	Log::info(LOG_MAIN, "current working dir: %s", workingDir.c_str());

	{
		Log::info(LOG_MAIN, "Entering mainloop");
		SDLMainLoop sdlBackend;
		sdlBackend.mainLoop(argc, argv);
		Log::info(LOG_MAIN, "Leaving mainloop");
	}

	Log::info(LOG_MAIN, "quit");
	return EXIT_SUCCESS;
}
