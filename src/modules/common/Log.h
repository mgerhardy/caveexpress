#pragma once

#include "common/IConsole.h"
#include "common/String.h"
#include "common/Common.h"
#include <vector>
#include <cstdio>
#include <memory>
#include <SDL_log.h>

typedef enum {
	LOG_CAMPAIGN,
	LOG_CLIENT,
	LOG_COMMON,
	LOG_GAME,
	LOG_GFX,
	LOG_MAIN,
	LOG_NETWORK,
	LOG_PARTICLES,
	LOG_SERVER,
	LOG_SERVICE,
	LOG_SOUND,
	LOG_SPRITES,
	LOG_TEXTURES,
	LOG_UI,
	LOG_GAMEIMPL,

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
