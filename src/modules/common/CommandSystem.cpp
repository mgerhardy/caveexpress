#include "CommandSystem.h"
#include "common/ConfigManager.h"
#include "common/Log.h"

#define CMD_LISTCOMMANDS "listcommands"

class CmdListCommands: public ICommand {
public:
	void run (const Args& args)
	{
		for (CommandList::const_iterator i = CommandSystem::get()._commands.begin();
				i != CommandSystem::get()._commands.end(); ++i) {
			Log::info2(LOG_COMMANDS, "%s", i->first.c_str());
		}
	}
};

CommandSystem::CommandSystem ()
{
	registerCommand(CMD_LISTCOMMANDS, new CmdListCommands());
	registerAlias("?", CMD_LISTCOMMANDS);
	registerAlias("help", CMD_LISTCOMMANDS);
}

CommandSystem::~CommandSystem ()
{
	_commands.clear();
}

void CommandSystem::registerAlias (const std::string& id, const std::string& command)
{
	Log::info2(LOG_COMMANDS, "register alias %s", id.c_str());
	_alias[id] = command;
}

CommandPtr CommandSystem::registerCommand (const std::string& id, ICommand* command)
{
	Log::info2(LOG_COMMANDS, "register command %s", id.c_str());
	CommandPtr ptr(command);
	_commands[id] = ptr;
	return ptr;
}

void CommandSystem::removeCommand (const std::string& id)
{
	_commands[id] = CommandPtr();
	auto i = _alias.find(id);
	if (i != _alias.end())
		_alias.erase(i);
}

ICommand* CommandSystem::getCommand (const std::string& command) const
{
	std::string id = command;
	// check whether it's an alias
	auto i = _alias.find(id);
	if (i != _alias.end())
		id = i->second;
	// Look up candidate in the map and return it if found
	const CommandList::const_iterator it = _commands.find(id);
	if (it != _commands.end()) {
		return it->second.get();
	}

	return nullptr;
}

void CommandSystem::executeCommandLine (const std::string& command) const
{
	if (command.empty())
		return;
	std::vector<String> commands = String(command).split(";");
	for (std::vector<String>::iterator i = commands.begin(); i != commands.end(); ++i) {
		std::vector<String> tokens = i->split();
		String cmd = tokens[0].eraseAllSpaces();
		tokens.erase(tokens.begin());
		executeCommand(cmd, tokens);
	}
}

bool CommandSystem::executeCommand (const std::string& command, ICommand::Args args) const
{
	std::string command2;
	if (command[0] == '+') {
		command2 = command.substr(1, command.length() - 1);
	} else {
		command2 = command;
	}
	ICommand* callback = getCommand(command2);
	if (callback != nullptr) {
		Log::debug2(LOG_COMMANDS, "run command %s with %i parameters", command.c_str(), (int)args.size());
		callback->run(args);
		return true;
	}

	Log::error(LOG_COMMANDS, "unknown command given: " + command2);

	return false;
}

void CommandSystem::getCommandNameList (std::vector<std::string> &commands) const
{
	for (CommandList::const_iterator i = _commands.begin(); i != _commands.end(); ++i) {
		commands.push_back(i->first);
	}
}
