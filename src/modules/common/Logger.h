#pragma once

#include "common/IConsole.h"
#include "common/String.h"
#include "common/Common.h"
#include <vector>

typedef enum {
	LOG_BACKEND,
	LOG_CLIENT,
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

	LOG_MAX
} LogCategory;

class Logger {
private:
	Logger ();

	typedef std::vector<IConsole*> Consoles;
	typedef Consoles::const_iterator ConsolesConstIter;
	typedef Consoles::iterator ConsolesIter;
	Consoles _consoles;
public:
	virtual ~Logger ();

	static Logger& get ();

	void addConsole (IConsole* console);
	void removeConsole (IConsole* console);

	void logInfo (LogCategory category, const std::string &string) const;
	void logError (LogCategory category, const std::string &string) const;
	void logDebug (LogCategory category, const std::string &string) const;
	void logTrace (LogCategory category, const std::string &string) const;
};

#define info(category, msg) Logger::get().logInfo(category, msg)
#define debug(category, msg) Logger::get().logDebug(category, msg)
#define trace(category, msg) Logger::get().logTrace(category, msg)
#define error(category, msg) Logger::get().logError(category, msg)
#define o(msg) Logger::get().logInfo(LOG_GENERAL, msg)
