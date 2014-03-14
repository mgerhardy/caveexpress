#pragma once

#include "engine/common/ICommand.h"
#include <SDL.h>

class CmdQuit: public ICommand {
private:
	SDL_Event _event;
	bool &_running;

public:
	CmdQuit (bool &running) :
			_running(running)
	{
		_event.type = SDL_QUIT;
	}

	void run (const Args& args) override
	{
		_running = false;
		SDL_PushEvent(&_event);
	}
};

