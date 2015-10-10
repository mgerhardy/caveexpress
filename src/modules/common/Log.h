#pragma once

#include "common/IConsole.h"
#include "common/String.h"
#include "common/Common.h"
#include <vector>
#include <cstdio>
#include <memory>
#include <SDL_log.h>

typedef enum {
	LOG_BACKEND,
	LOG_CLIENT,
	LOG_GFX,
	LOG_SERVER,
	LOG_MAIN,
	LOG_NET,
	LOG_MAP,
	LOG_THREAD,
	LOG_CAMPAIGN,
	LOG_COMMANDS,
	LOG_CONFIG,
	LOG_DEBUG,
	LOG_FILE,
	LOG_LUA,
	LOG_STORAGE,
	LOG_SYSTEM,
	LOG_GENERAL,
	LOG_UI,

	LOG_MAX
} LogCategory;

enum class LogLevel {
	LEVEL_TRACE,
	LEVEL_DEBUG,
	LEVEL_INFO,
	LEVEL_WARN,
	LEVEL_ERROR,
	LEVEL_MAX
};

struct LogLevelList {
	const char *logLevelStr;
	LogLevel logLevel;
	SDL_LogPriority sdlLevel;
};

extern LogLevelList LogLevels[];

class Log {
private:
	Log ();

	typedef std::vector<IConsole*> Consoles;
	typedef Consoles::const_iterator ConsolesConstIter;
	typedef Consoles::iterator ConsolesIter;
	Consoles _consoles;

	void vsnprint(LogLevel logLevel, LogCategory category, const char* msg, va_list args);
public:
	virtual ~Log ();

	static Log& get ();

	void addConsole (IConsole* console);
	void removeConsole (IConsole* console);

	static void trace(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));
	static void debug(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));
	static void info(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));
	static void warn(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));
	static void error(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));
};
