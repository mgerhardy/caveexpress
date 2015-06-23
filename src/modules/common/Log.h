#pragma once

#include "common/IConsole.h"
#include "common/String.h"
#include "common/Common.h"
#include <vector>
#include <cstdio>
#include <memory>

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

enum class LogLevel {
	LEVEL_TRACE,
	LEVEL_DEBUG,
	LEVEL_INFO,
	LEVEL_WARN,
	LEVEL_ERROR,
	LEVEL_MAX
};

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
	static void debug2(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));
	static void info2(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));
	static void warn(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));
	static void error2(LogCategory category, const char* msg, ...) __attribute__((format(printf, 2, 3)));

	static void debug(LogCategory category, const std::string& msg) {
		debug2(category, "%s", msg.c_str());
	}
	static void info(LogCategory category, const std::string& msg) {
		info2(category, "%s", msg.c_str());
	}
	static void error(LogCategory category, const std::string& msg) {
		error2(category, "%s", msg.c_str());
	}
};
