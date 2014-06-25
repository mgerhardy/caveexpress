#include "Logger.h"
#include "engine/common/ConfigManager.h"
#include <sstream>
#include <SDL_platform.h>
#ifdef __ANDROID__
#include <android/log.h>
#include "engine/common/Version.h"
#endif

char const* const loggerTypes[] = {
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
CASSERT(lengthof(loggerTypes) == LOG_MAX);

Logger::Logger ()
{
}

Logger::~Logger ()
{
	_consoles.clear();
}

Logger& Logger::get ()
{
	static Logger _logger;
	return _logger;
}

void Logger::removeConsole (IConsole* console)
{
	for (ConsolesIter i = _consoles.begin(); i != _consoles.end(); ++i) {
		if (*i == console) {
			_consoles.erase(i);
			break;
		}
	}
}

void Logger::addConsole (IConsole* console)
{
	_consoles.push_back(console);
}

void Logger::logInfo (LogCategory category, const std::string &string) const
{
	if (string.empty())
		return;

	std::stringstream ss;
	ss << "INFO: (";
	ss << loggerTypes[category] << ") ";
	ss << string;
	const std::string& message = ss.str();

	for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
		(*i)->logInfo(message);
	}

	if (_consoles.empty()) {
#ifdef __ANDROID__
		const std::string incCat = "(" + std::string(loggerTypes[category]) + ") " + string;
		__android_log_write(ANDROID_LOG_INFO, APPFULLNAME, incCat.c_str());
#else
		fprintf(stdout, "%s\n", message.c_str());
		fflush(stdout);
#endif
	}
}

void Logger::logError (LogCategory category, const std::string &string) const
{
	if (string.empty())
		return;
	std::stringstream ss;
	ss << "ERROR: (";
	ss << loggerTypes[category] << ") ";
	ss << string;
	const std::string& message = ss.str();

	for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
		(*i)->logError(message);
	}

	if (_consoles.empty()) {
#ifdef __ANDROID__
		const std::string incCat = "(" + std::string(loggerTypes[category]) + ") " + string;
		__android_log_write(ANDROID_LOG_ERROR, APPFULLNAME, incCat.c_str());
#else
		fprintf(stderr, "%s\n", message.c_str());
		fflush(stderr);
#endif
	}
}

void Logger::logDebug (LogCategory category, const std::string &string) const
{
#ifndef __NACL__
	if (!Config.isDebug())
		return;
#endif
	if (string.empty())
		return;

	std::stringstream ss;
	ss << "DEBUG: (";
	ss << loggerTypes[category] << ") ";
	ss << string;
	const std::string& message = ss.str();

	for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
		(*i)->logDebug(message);
	}

	if (_consoles.empty()) {
#ifdef __ANDROID__
		const std::string incCat = "(" + std::string(loggerTypes[category]) + ") " + string;
		__android_log_write(ANDROID_LOG_DEBUG, APPFULLNAME, incCat.c_str());
#else
		fprintf(stdout, "%s\n", message.c_str());
		fflush(stdout);
#endif
	}
}
