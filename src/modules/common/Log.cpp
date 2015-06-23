#include "common/Log.h"
#include "common/ConfigManager.h"
#include "common/System.h"
#include <SDL.h>

char const* const LogTypes[] = {
	"LOG_BACKEND",
	"LOG_CLIENT",
	"LOG_SERVER",
	"LOG_MAIN",
	"LOG_NET",
	"LOG_MAP",
	"LOG_THREAD",
	"LOG_CAMPAIGN",
	"LOG_COMMANDS",
	"LOG_CONFIG",
	"LOG_DEBUG",
	"LOG_FILE",
	"LOG_LUA",
	"LOG_STORAGE",
	"LOG_SYSTEM",
	"LOG_GENERAL"
};
CASSERT(lengthof(LogTypes) == LOG_MAX);

char const* const LogLevels[] = {
	"TRACE",
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR"
};
CASSERT(lengthof(LogLevels) == static_cast<int>(LogLevel::LEVEL_MAX));

Log::Log ()
{
}

Log::~Log ()
{
	_consoles.clear();
}

Log& Log::get ()
{
	static Log _Log;
	return _Log;
}

void Log::removeConsole (IConsole* console)
{
	for (ConsolesIter i = get()._consoles.begin(); i != get()._consoles.end(); ++i) {
		if (*i == console) {
			get()._consoles.erase(i);
			break;
		}
	}
}

void Log::addConsole (IConsole* console)
{
	_consoles.push_back(console);
}

void Log::vsnprint(LogLevel logLevel, LogCategory category, const char* msg, va_list args) {
	char buf[1024];
	const char *categoryStr = LogTypes[static_cast<int>(category)];
	const char *logLevelStr = LogLevels[static_cast<int>(logLevel)];
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = 0;
	SDL_Log("%s (%s): %s\n", logLevelStr, categoryStr, buf);

	switch (logLevel) {
		case LogLevel::LEVEL_INFO:
		case LogLevel::LEVEL_WARN:
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logInfo(buf);
			}
			break;
		case LogLevel::LEVEL_ERROR:
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logError(buf);
			}
			break;
		case LogLevel::LEVEL_DEBUG:
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logDebug(buf);
			}
			break;
		case LogLevel::LEVEL_TRACE:
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logTrace(buf);
			}
			break;
		default:
			break;
	}
}

void Log::info2 (LogCategory category, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_INFO, category, msg, args);
	va_end(args);
}

void Log::error2 (LogCategory category, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_ERROR, category, msg, args);
	va_end(args);
}

void Log::trace (LogCategory category, const char* msg, ...)
{
	if (!Config.isTrace())
		return;
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_TRACE, category, msg, args);
	va_end(args);
}

void Log::debug2 (LogCategory category, const char* msg, ...)
{
	if (!Config.isDebug())
		return;
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_DEBUG, category, msg, args);
	va_end(args);
}

void Log::warn (LogCategory category, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_WARN, category, msg, args);
	va_end(args);
}
