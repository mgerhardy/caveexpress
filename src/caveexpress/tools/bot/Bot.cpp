#include "BotBackend.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Logger.h"
#include "engine/common/System.h"
#include <SDL_main.h>

static BotBackend backend;

extern "C" int main (int argc, char *argv[])
{
	backend.mainLoop(argc, argv);
	::exit(EXIT_SUCCESS);
}
