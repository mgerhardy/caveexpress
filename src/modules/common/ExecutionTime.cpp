#include "ExecutionTime.h"

#include <SDL.h>
#include "common/Log.h"

#ifdef _WIN32
#ifdef USE_EXECTIME

#include <windows.h>

#define CLOCK_REALTIME 0

static LARGE_INTEGER getFILETIMEoffset ()
{
	SYSTEMTIME s;
	FILETIME f;
	LARGE_INTEGER t;

	s.wYear = 1970;
	s.wMonth = 1;
	s.wDay = 1;
	s.wHour = 0;
	s.wMinute = 0;
	s.wSecond = 0;
	s.wMilliseconds = 0;
	SystemTimeToFileTime(&s, &f);
	t.QuadPart = f.dwHighDateTime;
	t.QuadPart <<= 32;
	t.QuadPart |= f.dwLowDateTime;
	return (t);
}

static int clock_gettime (int X, struct timeval *tv)
{
	LARGE_INTEGER t;
	FILETIME f;
	double microseconds;
	static LARGE_INTEGER offset;
	static double frequencyToMicroseconds;
	static int initialized = 0;
	static BOOL usePerformanceCounter = 0;

	if (!initialized) {
		LARGE_INTEGER performanceFrequency;
		initialized = 1;
		usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
		if (usePerformanceCounter) {
			QueryPerformanceCounter(&offset);
			frequencyToMicroseconds = (double) performanceFrequency.QuadPart / 1000000.;
		} else {
			offset = getFILETIMEoffset();
			frequencyToMicroseconds = 10.;
		}
	}
	if (usePerformanceCounter) {
		QueryPerformanceCounter(&t);
	} else {
		GetSystemTimeAsFileTime(&f);
		t.QuadPart = f.dwHighDateTime;
		t.QuadPart <<= 32;
		t.QuadPart |= f.dwLowDateTime;
	}

	t.QuadPart -= offset.QuadPart;
	microseconds = (double) t.QuadPart / frequencyToMicroseconds;
	t.QuadPart = microseconds;
	tv->tv_sec = t.QuadPart / 1000000;
	tv->tv_usec = t.QuadPart % 1000000;
	return 0;
}
#endif
#endif

#ifdef USE_EXECTIME
inline std::string ExecutionTime::format (const char *msg, ...) const
{
	va_list ap;
	const std::size_t size = 1024;
	char text[size];

	va_start(ap, msg);
	SDL_vsnprintf(text, size, msg, ap);
	va_end(ap);

	return std::string(text);
}

inline void ExecutionTime::gettime (timespec &ts) const
{
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	ts.tv_sec = mts.tv_sec;
	ts.tv_nsec = mts.tv_nsec;
#else
	clock_gettime(CLOCK_REALTIME, &ts);
#endif
}

inline timespec ExecutionTime::diff (const timespec &start, const timespec &end) const
{
	timespec temp;
	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}
#endif

ExecutionTime::ExecutionTime (const std::string& id, long microDelay) :
		_id(id)
#ifdef USE_EXECTIME
		,_microDelay(microDelay)
#endif
{
#ifdef USE_EXECTIME
	_start.tv_sec = 0;
	_start.tv_nsec = 0;
	gettime(_start);
#else
	(void)microDelay;
#endif
}

ExecutionTime::~ExecutionTime ()
{
#ifdef USE_EXECTIME
	timespec end;
	end.tv_sec = 0;
	end.tv_nsec = 0;
	gettime(end);
	timespec d = diff(_start, end);
	if (_microDelay == 0L || d.tv_nsec / 1000 > _microDelay)
		Log::debug(LOG_DEBUG, "%s: %i,%09is", _id.c_str(), (int)d.tv_sec, (int)d.tv_nsec);
#endif
}
