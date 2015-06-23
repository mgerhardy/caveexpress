#include "server/SDLBackend.h"
#include "common/ConfigManager.h"
#include "common/Log.h"
#include "common/Application.h"
#include "common/System.h"
#include "GameRegistry.h"
#include <SDL.h>
#include <SDL_main.h>


extern "C" int main (int argc, char *argv[])
{
	Application& app = Singleton<Application>::getInstance();
	const GamePtr& game = Singleton<GameRegistry>::getInstance().getGame();
	app.setOrganisation("caveproductions");
	app.setName(game->getName());
#ifdef VERSION
	app.setVersion( VERSION );
#endif

	getSystem().init();
	const std::string workingDir = getSystem().getCurrentWorkingDir();
	Log::info2(LOG_MAIN, "current working dir: %s", workingDir.c_str());

	{
		Log::info2(LOG_MAIN, "Entering mainloop");
		SDLBackend sdlBackend;
		sdlBackend.mainLoop(argc, argv);
		Log::info2(LOG_MAIN, "Leaving mainloop");
	}

	Log::info2(LOG_MAIN, "quit");
	return EXIT_SUCCESS;
}
