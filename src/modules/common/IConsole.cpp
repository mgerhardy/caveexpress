#include "IConsole.h"
#include "common/ConfigManager.h"
#include "common/CommandSystem.h"
#include "common/FileSystem.h"
#include "common/Log.h"

IConsole::IConsole () :
		_commandLine(""), _overwrite(false), _frame(0), _cursorBlink(false), _cursorPos(0), _historyPos(0), _active(true)
{
}

void IConsole::executeCommandLine ()
{
	if (_commandLine.empty())
		return;

	_history.push_back(_commandLine);
	_historyPos = _history.size();

	std::vector<std::string> commands;
	string::splitString(_commandLine, commands, ";");

	for (const std::string& command : commands) {
		std::vector<std::string> tokens;
		string::splitString(command, tokens);
		std::string cmd = string::eraseAllSpaces(tokens[0]);
		tokens.erase(tokens.begin());
		if (!Commands.commandExists(cmd)) {
			ConfigVarPtr c = Config.getConfigVar(cmd, "", false);
			if (c) {
				if (tokens.empty()) {
					if (c->getValue().empty())
						Log::info(LOG_COMMON, "%s: no value set", cmd.c_str());
					else
						Log::info(LOG_COMMON, "%s: %s", cmd.c_str(), c->getValue().c_str());
				} else {
					c->setValue(string::eraseAllSpaces(tokens[0]));
				}
			} else {
				Log::info(LOG_COMMON, "unknown config variable %s", cmd.c_str());
			}
		} else {
			Commands.executeCommand(cmd, tokens);
		}
	}
	_commandLine.clear();
	_cursorPos = 0;
}

bool IConsole::onTextInput (const std::string& text)
{
	if (!_active)
		return false;

	if (_overwrite)
		cursorDelete();
	_commandLine.insert(_commandLine.begin() + _cursorPos, text.begin(), text.end());
	_cursorPos = _commandLine.size();
	return true;
}

void IConsole::cursorLeft ()
{
	if (_cursorPos > 0)
		_cursorPos--;
}

void IConsole::cursorUp ()
{
	if (_historyPos <= 0)
		return;

	--_historyPos;
	_commandLine = _history[_historyPos];
	_cursorPos = _commandLine.size();
}

void IConsole::cursorDown ()
{
	++_historyPos;

	const int entries = _history.size();
	if (_historyPos >= entries) {
		_historyPos = entries;
		_commandLine = "";
		_cursorPos = 0;
		return;
	}
	_commandLine = _history[_historyPos];
	_cursorPos = _commandLine.size();
}

void IConsole::cursorRight ()
{
	const int size = _commandLine.size();
	if (_cursorPos < size)
		_cursorPos++;
}

void IConsole::autoComplete ()
{
	std::vector<std::string> matches;
	std::vector<std::string> strings;
	std::string match = "";
	string::splitString(_commandLine, strings);
	if (!strings.empty()) {
		ICommand* command = Commands.getCommand(strings[0]);
		if (command != nullptr) {
			if (strings.size() > 1)
				match = strings.back();
			command->complete(match, matches);
		}
	}
	if (matches.empty()) {
		std::vector<std::string> commands;
		Commands.getCommandNameList(commands);
		for (const std::string& name : commands) {
			if (_commandLine.empty() || name.substr(0, _commandLine.size()) == _commandLine) {
				matches.push_back(name);
			}
		}
		Config.autoComplete(_commandLine, matches);
	}

	if (matches.size() == 1) {
		if (strings.size() == 1) {
			_commandLine = matches.front();
		} else {
			const int start = _commandLine.size() - match.size();
			_commandLine.replace(start, _commandLine.size() - 1, matches.front());
		}
		_cursorPos = _commandLine.size();
	} else {
		for (std::vector<std::string>::const_iterator i = matches.begin(); i != matches.end(); ++i) {
			Log::info(LOG_COMMON, "%s", (*i).c_str());
		}
	}
}

void IConsole::cursorDelete (bool moveCursor)
{
	if (_commandLine.empty())
		return;
	const int size = _commandLine.size();
	if (moveCursor || _cursorPos > size - 1)
		cursorLeft();
	_commandLine.erase(_cursorPos, 1);
}

FileConsole::FileConsole() {
	const std::string path = FS.getAbsoluteWritePath() + "logfile.txt";
	SDL_RWops *rwops = FS.createRWops(path, "w");
	_filePtr = FilePtr(new File(rwops, path));
	_filePtr->writeString("LOGFILE");
	Log::get().addConsole(this);
}

FileConsole::~FileConsole() {
	Log::get().removeConsole(this);
}

void FileConsole::logInfo(const std::string& string) {
	_filePtr->appendString(string.c_str());
}

void FileConsole::logError(const std::string& string) {
	logInfo(string);
}

void FileConsole::logDebug(const std::string& string) {
	logInfo(string);
}
