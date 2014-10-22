#include "engine/server/SDLBackend.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Logger.h"
#include "engine/common/Application.h"
#include "engine/common/System.h"
#include "engine/GameRegistry.h"
#include <SDL.h>
#include <SDL_main.h>


extern "C" int main (int argc, char *argv[])
{
	Application& app = Singleton<Application>::getInstance();
	const GamePtr& game = Singleton<GameRegistry>::getInstance().getGame();
	app.setOrganisation("caveproductions");
	app.setName(game->getName());

	getSystem().init();
	const std::string workingDir = getSystem().getCurrentWorkingDir();
	info(LOG_MAIN, "current working dir: " + workingDir);

	{
		info(LOG_MAIN, "Entering mainloop");
		SDLBackend sdlBackend;
		sdlBackend.mainLoop(argc, argv);
		info(LOG_MAIN, "Leaving mainloop");
	}

	info(LOG_MAIN, "quit");
	return EXIT_SUCCESS;
}
