#pragma once

#include "common/Log.h"
#include <unordered_set>
#include <SDL.h>

namespace cavepacker {

#define TIMEOUTREACHED(millis) \
	if (SDL_TICKS_PASSED(SDL_GetTicks(), millis)) { \
		Log::trace(LOG_GAMEIMPL, "abort deadlock detection, timeout reached"); \
		return false; \
	}

typedef std::unordered_set<int> DeadlockSet;
class BoardState;

}
