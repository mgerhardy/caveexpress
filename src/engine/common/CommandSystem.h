#pragma once

#include "engine/common/ICommand.h"
#include "engine/common/NonCopyable.h"
#include <unordered_map>
#include <string>
#include <vector>

class CmdListCommands;
typedef std::unordered_map<std::string, CommandPtr> CommandList;

class CommandSystem: public NonCopyable {
	friend class CmdListCommands;
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

	CommandPtr registerCommand (const std::string& id, ICommand* command);
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
