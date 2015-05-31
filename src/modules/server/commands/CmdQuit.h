#pragma once

#include "common/ICommand.h"
#include <SDL.h>

class CmdQuit: public ICommand {
public:
	void run (const Args& args) override {
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
	}
};

