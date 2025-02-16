#pragma once

#include "common/Config.h"
#include "common/ports/ISystem.h"
#include <SDL_platform.h>

#ifdef __ANDROID__
#include "common/ports/Android.h"
#elif defined __WIN32__
#include "common/ports/Windows.h"
#elif defined __IPHONEOS__
#include "common/ports/IOS.h"
#elif defined __MACOSX__
#include "common/ports/Darwin.h"
#elif defined EMSCRIPTEN
#include "common/ports/HTML5.h"
#else
#include "common/ports/Unix.h"
#endif

inline ISystem& getSystem ()
{
#ifdef __WIN32__
	static Windows _system;
#elif defined __ANDROID__
	static Android _system;
#elif defined __IPHONEOS__
	static IOS _system;
#elif defined __MACOSX__
	static Darwin _system;
#elif defined EMSCRIPTEN
	static HTML5 _system;
#else
	static Unix _system;
#endif
	return _system;
}

#define System getSystem()

#ifdef HAVE_EXECINFO_H
namespace {
	inline void globalSignalHandler (int s) {
		getSystem().signalHandler(s);
	}
}
#endif
