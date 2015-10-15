#pragma once

#include <string>
#include <cstring>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstdarg>
#include <stdio.h>
#include <cstdlib>
#include <cstddef>
#include <SDL.h>
#ifdef __ANDROID__
#include <sstream>
#endif

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#if SDL_COMPILEDVERSION <= SDL_VERSIONNUM(2, 0, 3)
#define SDL_PRINTF_FORMAT_STRING
#define SDL_PRINTF_VARARG_FUNC(...)
#endif

namespace string {

template<class T>
inline std::string toString (const T& t)
{
#ifdef __ANDROID__
	std::ostringstream os ;
	os << t ;
	return os.str() ;
#else
	return std::to_string(t);
#endif
}

inline int toInt (const std::string& str, int defaultValue = 0)
{
	if (str.empty())
		return defaultValue;
	return atoi(str.c_str());
}

template<class T>
inline std::string toBitString (T type)
{
	const size_t size = sizeof(T);
	std::string bits;
	bits.reserve(size * 4);
	const uint8_t* raw = (const uint8_t*)&type;
	for (size_t i = 0; i < size; ++i) {
		const uint8_t byte = raw[i];
		for (int b = 0; b < sizeof(uint8_t); ++b) {
			const int bit = 1 << b;
			if (byte & bit)
				bits.append("1");
			else
				bits.append("0");
		}
	}
	return bits;
}

inline bool toBool (const std::string& str)
{
	return str == "1" || str == "true";
}

inline float toFloat (const std::string& str, float defaultValue = 0.0f)
{
	if (str.empty())
		return defaultValue;
	return atof(str.c_str());
}

inline std::string cutBeforeLastMatch (const std::string& _string, const std::string& pattern)
{
	std::string::size_type pos = _string.rfind(pattern);
	if (pos == std::string::npos)
		return _string;
	return _string.substr(pos + 1);
}

inline bool matches (const std::string& _string, const std::string& text)
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
		}
	}
	return *t == '\0';
}

inline void splitString (const std::string& string, std::vector<std::string>& tokens, const std::string& delimiters = " \t\r\n\f\v")
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = string.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = string.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(string.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = string.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = string.find_first_of(delimiters, lastPos);
	}
}

inline std::string toLower (const std::string& string)
{
	std::string convert = string;
	std::transform(convert.begin(), convert.end(), convert.begin(),
			(int(*) (int)) std::tolower);
	return convert;
}

inline bool startsWith (const std::string& string, const std::string& token)
{
	return !string.compare(0, token.size(), token);
}

inline bool endsWith (const std::string& string, const std::string& end)
{
	const std::size_t strLength = string.length();
	const std::size_t endLength = end.length();
	if (strLength >= endLength) {
		const std::size_t index = strLength - endLength;
		return string.compare(index, endLength, end) == 0;
	}
	return false;
}

std::string format (SDL_PRINTF_FORMAT_STRING const char *msg, ...) SDL_PRINTF_VARARG_FUNC(1);
inline std::string format (const char *msg, ...)
{
	va_list ap;
	const std::size_t bufSize = 1024;
	char text[bufSize];

	va_start(ap, msg);
	SDL_vsnprintf(text, bufSize, msg, ap);
	va_end(ap);

	return std::string(text);
}


inline std::string replaceAll (const std::string& str, const std::string& searchStr, const std::string& replaceStr)
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

inline std::string cutAfterFirstMatch (const std::string& str, const std::string& pattern, size_t start = 0)
{
	std::string::size_type pos = str.find_first_of(pattern, 0);
	return str.substr(start, pos);
}

inline std::string eraseAllSpaces (const std::string& str)
{
	std::string tmp = str;
	tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
	return tmp;
}

inline bool contains (const std::string& str, const std::string& search)
{
	return str.rfind(search) != std::string::npos;
}

inline std::string ltrim(const std::string &str) {
	size_t startpos = str.find_first_not_of(" \t");
	if (std::string::npos != startpos) {
		return str.substr(startpos);
	}
	return str;
}

inline std::string rtrim(const std::string &str) {
	size_t endpos = str.find_last_not_of(" \t");
	if (std::string::npos != endpos) {
		return str.substr(0, endpos + 1);
	}
	return str;
}

inline std::string trim(const std::string &str) {
	return ltrim(rtrim(str));
}
}
