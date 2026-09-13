#pragma once

#include "common/String.h"
#include <cstring>
#include <string>
#include <vector>

namespace StartPositionModes {

enum Type {
	BOTH = 0,
	SINGLEPLAYER = 1,
	MULTIPLAYER = 2
};

inline const char* toString (Type mode)
{
	switch (mode) {
	case SINGLEPLAYER:
		return "singleplayer";
	case MULTIPLAYER:
		return "multiplayer";
	case BOTH:
	default:
		return "both";
	}
}

inline const char* shortLabel (Type mode)
{
	switch (mode) {
	case SINGLEPLAYER:
		return "SP";
	case MULTIPLAYER:
		return "MP";
	case BOTH:
	default:
		return "B";
	}
}

inline Type fromString (const std::string& value)
{
	const std::string mode = string::toLower(string::trim(value));
	if (mode == "singleplayer" || mode == "single" || mode == "sp")
		return SINGLEPLAYER;
	if (mode == "multiplayer" || mode == "multi" || mode == "mp")
		return MULTIPLAYER;
	return BOTH;
}

inline bool allowsSingleplayer (Type mode)
{
	return mode == BOTH || mode == SINGLEPLAYER;
}

inline bool allowsMultiplayer (Type mode)
{
	return mode == BOTH || mode == MULTIPLAYER;
}

inline bool allows (Type mode, bool multiplayer)
{
	return multiplayer ? allowsMultiplayer(mode) : allowsSingleplayer(mode);
}

inline bool parseStartModesLine (const std::string& line, std::vector<Type>& modes)
{
	const std::string lower = string::toLower(line);
	const size_t pos = lower.find("startmodes:");
	if (pos == std::string::npos)
		return false;
	const std::string rest = line.substr(pos + std::strlen("startmodes:"));
	std::vector<std::string> tokens;
	string::splitString(rest, tokens, ",");
	modes.clear();
	for (const std::string& token : tokens) {
		const std::string trimmed = string::trim(token);
		if (!trimmed.empty())
			modes.push_back(fromString(trimmed));
	}
	return !modes.empty();
}

inline int countMultiplayerStartsFromLua (const std::string& source)
{
	int count = 0;
	size_t pos = 0;
	while ((pos = source.find("addStartPosition", pos)) != std::string::npos) {
		const size_t open = source.find('(', pos);
		if (open == std::string::npos)
			break;
		const size_t close = source.find(')', open);
		if (close == std::string::npos)
			break;
		const std::string args = source.substr(open + 1, close - open - 1);
		std::vector<std::string> strs;
		for (size_t i = 0; i < args.size(); ++i) {
			if (args[i] != '"' && args[i] != '\'')
				continue;
			const char quote = args[i];
			const size_t end = args.find(quote, i + 1);
			if (end == std::string::npos)
				break;
			strs.push_back(args.substr(i + 1, end - i - 1));
			i = end;
		}
		Type mode = BOTH;
		if (strs.size() >= 3)
			mode = fromString(strs[2]);
		if (allowsMultiplayer(mode))
			++count;
		pos = close + 1;
	}
	return count;
}

template<typename Starts>
inline void applyStartModes (Starts& starts, const std::vector<Type>& modes)
{
	for (size_t i = 0; i < starts.size() && i < modes.size(); ++i)
		starts[i]._mode = modes[i];
}

inline int countMultiplayerStartsFromSokoban (const std::string& source)
{
	std::vector<Type> modes;
	int boardPlayers = 0;
	size_t lineStart = 0;
	while (lineStart <= source.size()) {
		size_t lineEnd = source.find('\n', lineStart);
		if (lineEnd == std::string::npos)
			lineEnd = source.size();
		const std::string line = source.substr(lineStart, lineEnd - lineStart);
		const std::string trimmed = string::trim(line);
		if (string::startsWith(trimmed, ";") || StartPositionModes::parseStartModesLine(line, modes)
				|| string::startsWith(string::toLower(trimmed), "title:")
				|| string::startsWith(string::toLower(trimmed), "collection:")) {
			StartPositionModes::parseStartModesLine(line, modes);
		} else {
			for (char c : line) {
				if (c == '@' || c == '+')
					++boardPlayers;
			}
		}
		if (lineEnd == source.size())
			break;
		lineStart = lineEnd + 1;
	}
	if (modes.empty())
		return boardPlayers;
	int count = 0;
	for (int i = 0; i < boardPlayers; ++i) {
		const Type mode = i < (int)modes.size() ? modes[i] : BOTH;
		if (allowsMultiplayer(mode))
			++count;
	}
	return count;
}

}
