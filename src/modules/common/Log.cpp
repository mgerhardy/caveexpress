#include "common/Log.h"
#include "common/ConfigManager.h"
#include "common/System.h"
#include <sstream>
#include <SDL_platform.h>

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

void Log::info (LogCategory category, const std::string &string)
{
	if (string.empty())
		return;

	std::stringstream ss;
	ss << "INFO: (";
	ss << LogTypes[category] << ") ";
	ss << string;
	const std::string& message = ss.str();

	Log& l = get();
	for (ConsolesConstIter i = l._consoles.begin(); i != l._consoles.end(); ++i) {
		(*i)->logInfo(message);
	}

	if (l._consoles.empty()) {
		System.logOutput(message);
	}
}

void Log::error (LogCategory category, const std::string &string)
{
	if (string.empty())
		return;
	std::stringstream ss;
	ss << "ERROR: (";
	ss << LogTypes[category] << ") ";
	ss << string;
	const std::string& message = ss.str();

	Log& l = get();
	for (ConsolesConstIter i = l._consoles.begin(); i != l._consoles.end(); ++i) {
		(*i)->logError(message);
	}

	if (l._consoles.empty()) {
		System.logError(message);
	}
}

void Log::trace (LogCategory category, const std::string &string)
{
	if (string.empty())
		return;

	std::stringstream ss;
	ss << "TRACE: (";
	ss << LogTypes[category] << ") ";
	ss << string;
	const std::string& message = ss.str();

	Log& l = get();
	for (ConsolesConstIter i = l._consoles.begin(); i != l._consoles.end(); ++i) {
		(*i)->logTrace(message);
	}

	if (l._consoles.empty()) {
		System.logOutput(message);
	}
}

void Log::debug (LogCategory category, const std::string &string)
{
	if (string.empty())
		return;

	std::stringstream ss;
	ss << "DEBUG: (";
	ss << LogTypes[category] << ") ";
	ss << string;
	const std::string& message = ss.str();

	Log& l = get();
	for (ConsolesConstIter i = l._consoles.begin(); i != l._consoles.end(); ++i) {
		(*i)->logDebug(message);
	}

	if (l._consoles.empty()) {
		System.logOutput(message);
	}
}
