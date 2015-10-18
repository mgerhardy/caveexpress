#include "common/Log.h"
#include "common/ConfigManager.h"
#include "common/System.h"
#include <SDL.h>

char const* const LogTypes[] = {
	"LOG_CAMPAIGN",
	"LOG_CLIENT",
	"LOG_COMMON",
	"LOG_GAME",
	"LOG_GFX",
	"LOG_MAIN",
	"LOG_NETWORK",
	"LOG_PARTICLES",
	"LOG_SERVER",
	"LOG_SERVICE",
	"LOG_SOUND",
	"LOG_SPRITES",
	"LOG_TEXTURES",
	"LOG_UI",
	"LOG_GAMEIMPL"
};
CASSERT(lengthof(LogTypes) == LOG_MAX);

#ifdef __LINUX__
#define ANSI_COLOR_RESET "\033[0m"
#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_GREEN "\033[32m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_BLUE "\033[34m"
#define ANSI_COLOR_CYAN "\033[36m"
#else
#define ANSI_COLOR_RESET ""
#define ANSI_COLOR_RED ""
#define ANSI_COLOR_GREEN ""
#define ANSI_COLOR_YELLOW ""
#define ANSI_COLOR_BLUE ""
#define ANSI_COLOR_CYAN ""
#endif

LogLevelList LogLevels[] = {
	{"TRACE", LogLevel::LEVEL_TRACE, SDL_LOG_PRIORITY_VERBOSE},
	{"DEBUG", LogLevel::LEVEL_DEBUG, SDL_LOG_PRIORITY_DEBUG},
	{"INFO",  LogLevel::LEVEL_INFO, SDL_LOG_PRIORITY_INFO},
	{"WARN",  LogLevel::LEVEL_WARN, SDL_LOG_PRIORITY_WARN},
	{"ERROR", LogLevel::LEVEL_ERROR, SDL_LOG_PRIORITY_ERROR},
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
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = '\0';

	switch (logLevel) {
		case LogLevel::LEVEL_INFO:
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_GREEN "(%s): %s" ANSI_COLOR_RESET "\n", categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logInfo(buf);
			}
			break;
		case LogLevel::LEVEL_WARN:
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_YELLOW "(%s): %s" ANSI_COLOR_RESET "\n", categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logInfo(buf);
			}
			break;
		case LogLevel::LEVEL_ERROR:
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_RED "(%s): %s" ANSI_COLOR_RESET "\n", categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logError(buf);
			}
			break;
		case LogLevel::LEVEL_DEBUG:
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_BLUE "(%s): %s" ANSI_COLOR_RESET "\n", categoryStr, buf);
			for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
				(*i)->logDebug(buf);
			}
			break;
		case LogLevel::LEVEL_TRACE:
			SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, ANSI_COLOR_CYAN "(%s): %s" ANSI_COLOR_RESET "\n", categoryStr, buf);
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
	if (Config.getLogLevel() != LogLevel::LEVEL_TRACE) {
		return;
	}
	va_list args;
	va_start(args, msg);
	get().vsnprint(LogLevel::LEVEL_TRACE, category, msg, args);
	va_end(args);
}

void Log::debug (LogCategory category, const char* msg, ...)
{
	LogLevel level = Config.getLogLevel();
	if (level != LogLevel::LEVEL_TRACE && level != LogLevel::LEVEL_DEBUG) {
		return;
	}
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
