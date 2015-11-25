#include "String.h"

namespace string {

std::string cutBeforeLastMatch (const std::string& _string, const std::string& pattern)
{
	std::string::size_type pos = _string.rfind(pattern);
	if (pos == std::string::npos)
		return _string;
	return _string.substr(pos + 1);
}

bool matches (const std::string& _string, const std::string& text)
{
	std::string::const_iterator p = _string.begin();
	std::string::const_iterator t = text.begin();
	char c;

	while ((c = *p++) != '\0') {
		switch (c) {
		case '*':
			return true;
		case '?':
			if (*t == '\0')
				return false;
			else
				++t;
			break;
		default:
			if (c != *t++)
				return false;
			break;
		}
	}
	return *t == '\0';
}

void splitString (const std::string& string, std::vector<std::string>& tokens, const std::string& delimiters)
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = string.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = string.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(string.substr(lastPos, pos - lastPos));
		// Skip delimiters.
		lastPos = string.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = string.find_first_of(delimiters, lastPos);
	}
}

bool endsWith (const std::string& string, const std::string& end)
{
	const std::size_t strLength = string.length();
	const std::size_t endLength = end.length();
	if (strLength >= endLength) {
		const std::size_t index = strLength - endLength;
		return string.compare(index, endLength, end) == 0;
	}
	return false;
}

std::string format (const char *msg, ...)
{
	va_list ap;
	static const std::size_t bufSize = 1024;
	char text[bufSize];

	va_start(ap, msg);
	SDL_vsnprintf(text, bufSize, msg, ap);
	va_end(ap);

	return std::string(text);
}

std::string replaceAll (const std::string& str, const std::string& searchStr, const std::string& replaceStr)
{
	if (str.empty())
		return str;
	std::string sNew = str;
	std::string::size_type loc;
	const std::string::size_type replaceLength = replaceStr.length();
	const std::string::size_type searchLength = searchStr.length();
	std::string::size_type lastPosition = 0;
	while (std::string::npos != (loc = sNew.find(searchStr, lastPosition))) {
		sNew.replace(loc, searchLength, replaceStr);
		lastPosition = loc + replaceLength;
	}
	return sNew;
}

}
