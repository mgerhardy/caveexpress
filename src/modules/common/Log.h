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

class Log {
private:
	Log ();

	typedef std::vector<IConsole*> Consoles;
	typedef Consoles::const_iterator ConsolesConstIter;
	typedef Consoles::iterator ConsolesIter;
	Consoles _consoles;
public:
	virtual ~Log ();

	static Log& get ();

	void addConsole (IConsole* console);
	void removeConsole (IConsole* console);

	static void info (LogCategory category, const std::string &string);
	static void error (LogCategory category, const std::string &string);
	static void debug (LogCategory category, const std::string &string);
	static void trace (LogCategory category, const std::string &string);
};
