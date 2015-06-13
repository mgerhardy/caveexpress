#include "Logger.h"
#include "common/ConfigManager.h"
#include "common/System.h"
#include <sstream>
#include <SDL_platform.h>

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
		System.logOutput(message);
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
		System.logError(message);
	}
}

void Logger::logTrace (LogCategory category, const std::string &string) const
{
	if (string.empty())
		return;

	std::stringstream ss;
	ss << "TRACE: (";
	ss << loggerTypes[category] << ") ";
	ss << string;
	const std::string& message = ss.str();

	for (ConsolesConstIter i = _consoles.begin(); i != _consoles.end(); ++i) {
		(*i)->logTrace(message);
	}

	if (_consoles.empty()) {
		System.logOutput(message);
	}
}

void Logger::logDebug (LogCategory category, const std::string &string) const
{
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
		System.logOutput(message);
	}
}
