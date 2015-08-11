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
	"LOG_GENERAL",
	"LOG_UI"
};
CASSERT(lengthof(LogTypes) == LOG_MAX);

LogLevelList LogLevels[] = {
	{"TRACE", LogLevel::LEVEL_TRACE},
	{"DEBUG", LogLevel::LEVEL_DEBUG},
	{"INFO",  LogLevel::LEVEL_INFO},
	{"WARN",  LogLevel::LEVEL_WARN},
	{"ERROR", LogLevel::LEVEL_ERROR},
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
	const char *logLevelStr = LogLevels[static_cast<int>(logLevel)].logLevelStr;
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = 0;

	switch (logLevel) {
		case LogLevel::LEVEL_INFO:
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s (%s): %s\n", logLevelStr, categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logInfo(buf);
			}
			break;
		case LogLevel::LEVEL_WARN:
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s (%s): %s\n", logLevelStr, categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logInfo(buf);
			}
			break;
		case LogLevel::LEVEL_ERROR:
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s (%s): %s\n", logLevelStr, categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logError(buf);
			}
			break;
		case LogLevel::LEVEL_DEBUG:
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s (%s): %s\n", logLevelStr, categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logDebug(buf);
			}
			break;
		case LogLevel::LEVEL_TRACE:
			SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "%s (%s): %s\n", logLevelStr, categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logTrace(buf);
			}
			break;
		default:
			break;
	}
}

void Log::info (LogCategory category, const char* msg, ...)
{
	LogLevel level = Config.getLogLevel();
	if (level == LogLevel::LEVEL_ERROR || level == LogLevel::LEVEL_WARN)
		return;
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_INFO, category, msg, args);
	va_end(args);
}

void Log::error (LogCategory category, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_ERROR, category, msg, args);
	va_end(args);
}

void Log::trace (LogCategory category, const char* msg, ...)
{
	if (Config.getLogLevel() != LogLevel::LEVEL_TRACE)
		return;
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_TRACE, category, msg, args);
	va_end(args);
}

void Log::debug (LogCategory category, const char* msg, ...)
{
	LogLevel level = Config.getLogLevel();
	if (level != LogLevel::LEVEL_TRACE && level != LogLevel::LEVEL_DEBUG)
		return;
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_DEBUG, category, msg, args);
	va_end(args);
}

void Log::warn (LogCategory category, const char* msg, ...)
{
	LogLevel level = Config.getLogLevel();
	if (level == LogLevel::LEVEL_ERROR)
		return;
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_WARN, category, msg, args);
	va_end(args);
}
