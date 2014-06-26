#pragma once

#include "engine/common/Config.h"

#ifdef HAVE_CLOCKGETTIME
#ifdef DEBUG
// TODO: find a workaround for timespec
#ifndef _WIN32
#define USE_EXECTIME
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#endif
#endif
#endif

#include <string>

class ExecutionTime {
private:
	const std::string _id;
#ifdef USE_EXECTIME
	// either 0 - then the time is printed each time - or the amount of micro that the execution time may not exceed
	const long _microDelay;
	mutable timespec _start;
	std::string format (const char *msg, ...) const;
	inline void gettime (timespec &ts) const;
	inline timespec diff (const timespec &start, const timespec &end) const;
#endif

public:
	ExecutionTime (const std::string& id, long microDelay = 0L);
	~ExecutionTime ();
};
