#pragma once

#include "common/ICommand.h"
#include "common/NonCopyable.h"
#include "common/Log.h"
#include <unordered_map>
#include <string>
#include <vector>

typedef std::unordered_map<std::string, CommandPtr> CommandList;

class CommandSystem: public NonCopyable {
private:
	CommandSystem ();

	CommandList _commands;
	std::unordered_map<std::string, std::string> _alias;

public:
	virtual ~CommandSystem ();

	static CommandSystem& get ()
	{
		static CommandSystem commandSystem;
		return commandSystem;
	}

	ICommand* getCommand (const std::string& command) const;
	bool commandExists (const std::string& command) const;

	template<typename Func>
	CommandPtr registerCommand (const std::string& id, Func func) {
		Log::info(LOG_COMMON, "register command %s", id.c_str());
		auto ptr = CommandPtr(new CommandBindArgs(func));
		_commands[id] = ptr;
		return ptr;
	}

	CommandPtr registerCommandRaw (const std::string& id, ICommand* cmd) {
		Log::info(LOG_COMMON, "register command %s", id.c_str());
		auto ptr = CommandPtr(cmd);
		_commands[id] = ptr;
		return ptr;
	}

	template<typename Func>
	CommandPtr registerCommandString (const std::string& id, Func func) {
		Log::info(LOG_COMMON, "register command %s", id.c_str());
		auto ptr = CommandPtr(new CommandBindString(func));
		_commands[id] = ptr;
		return ptr;
	}

	template<typename Func>
	CommandPtr registerCommandVoid (const std::string& id, Func func) {
		Log::info(LOG_COMMON, "register command %s", id.c_str());
		auto ptr = CommandPtr(new CommandBindVoid(func));
		_commands[id] = ptr;
		return ptr;
	}

	void registerAlias (const std::string& id, const std::string& command);
	void removeCommand (const std::string& id);

	bool executeCommand (const std::string& command, ICommand::Args args = ICommand::Args(0)) const;
	void executeCommandLine (const std::string& command) const;

	void getCommandNameList (std::vector<std::string> &commands) const;
};

inline bool CommandSystem::commandExists (const std::string& command) const
{
	return getCommand(command) != nullptr;
}

#define Commands CommandSystem::get()
