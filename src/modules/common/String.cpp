#include "String.h"
#include <cctype>
#include <stdio.h>

String::String () :
		_string("")
{
}

String::String (const char *string, size_t _size) :
		_string(string, _size)
{
}

String::String (const char *string) :
		_string(string)
{
}

String::~String ()
{
}

String::String (const std::string &string) :
		_string(string)
{
}

String::String (const String &string) :
		_string(string._string)
{
}

bool String::startsWith (const std::string& string) const
{
	return !_string.compare(0, string.size(), string);
}

bool String::contains (const std::string& string) const
{
	return _string.rfind(string) != std::string::npos;
}

bool String::toBool (bool defaultValue) const
{
	if (_string.empty())
		return defaultValue;
	return _string == "1" || _string == "true";
}

int String::toInt (int defaultValue) const
{
	if (_string.empty())
		return defaultValue;
	return atoi(_string.c_str());
}

float String::toFloat (float defaultValue) const
{
	if (_string.empty())
		return defaultValue;
	return atof(_string.c_str());
}

String String::substr (std::size_t pos, std::size_t n) const
{
	return _string.substr(pos, n);
}

std::size_t String::rfind (String string) const
{
	return _string.rfind(string._string);
}

String String::toLower () const
{
	String convert = *this;
	std::transform(convert._string.begin(), convert._string.end(), convert._string.begin(),
			(int(*) (int)) std::tolower);
	return convert;
}

String String::toUpper () const
{
	String convert = *this;
	std::transform(convert._string.begin(), convert._string.end(), convert._string.begin(),
			(int(*) (int)) std::toupper);
	return convert;
}

void String::set (const std::string& string)
{
	_string = string;
}

const char *String::c_str () const
{
	return _string.c_str();
}

bool String::empty () const
{
	return _string.empty();
}

std::vector<String> String::split (const String& delimiters) const
{
	std::vector<String> tokens;
	// Skip delimiters at beginning.
	std::string::size_type lastPos = _string.find_first_not_of(delimiters._string, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = _string.find_first_of(delimiters._string, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(_string.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = _string.find_first_not_of(delimiters._string, pos);
		// Find next "non-delimiter"
		pos = _string.find_first_of(delimiters._string, lastPos);
	}
	return tokens;
}

String String::replaceAll (const String& searchStr, const String& replaceStr) const
{
	if (_string.empty())
		return *this;
	String sNew = *this;
	std::string::size_type loc;
	const std::string::size_type replaceLength = replaceStr._string.length();
	const std::string::size_type searchLength = searchStr._string.length();
	std::string::size_type lastPosition = 0;
	while (std::string::npos != (loc = sNew._string.find(searchStr._string, lastPosition))) {
		sNew._string.replace(loc, searchLength, replaceStr._string);
		lastPosition = loc + replaceLength;
	}
	return sNew;
}

bool String::endsWith (const String& end) const
{
	const std::size_t strLength = _string.length();
	const std::size_t endLength = end._string.length();
	if (strLength >= endLength) {
		const std::size_t index = strLength - endLength;
		return _string.compare(index, endLength, end._string) == 0;
	}
	return false;
}

String String::eraseAllSpaces ()
{
	String tmp = *this;
	tmp._string.erase(std::remove(tmp._string.begin(), tmp._string.end(), ' '), tmp._string.end());
	return tmp;
}

String String::cutAfterFirstMatch (const String& pattern, size_t start)
{
	std::string::size_type pos = _string.find_first_of(pattern._string, 0);
	if (pos == std::string::npos)
		return _string;
	return _string.substr(start, pos);
}

String String::cutBeforeLastMatch (const std::string& pattern)
{
	std::string::size_type pos = _string.rfind(pattern);
	if (pos == std::string::npos)
		return _string;
	return _string.substr(pos + 1);
}

String String::removeFromEnd (const String& pattern)
{
	size_t pos = rfind(pattern);
	if (pos == std::string::npos)
		return _string;
	return _string.substr(0, pos);
}

bool String::isUTF8Multibyte (char c)
{
	return (c & 0xc0) == 0x80;
}

size_t String::getUTF8LengthForCharacter (unsigned char c)
{
	if (c < 0x80)
		return 1;
	if (c < 0xc0)
		return 0;
	if (c < 0xe0)
		return 2;
	if (c < 0xf0)
		return 3;
	if (c < 0xf8)
		return 4;
	/* 5 and 6 byte sequences are no longer valid. */
	return 0;
}

size_t String::getUTF8LengthForInt (int c)
{
	if (c <= 0x7F)
		return 1;
	if (c <= 0x07FF)
		return 2;
	if (c <= 0xFFFF)
		return 3;
	if (c <= 0x10FFFF) /* highest defined Unicode code */
		return 4;
	return 0;
}

size_t String::getUTF8Length () const
{
	size_t result = 0;
	const char *string = _string.c_str();

	while (string[0] != '\0') {
		const int n = getUTF8LengthForCharacter((unsigned char) *string);
		string += n;
		result++;
	}
	return result;
}

String String::format (const char *msg, ...)
{
	va_list ap;
	const std::size_t bufSize = 1024;
	char text[bufSize];

	va_start(ap, msg);
	vsnprintf(text, bufSize, msg, ap);
	va_end(ap);

	return String(text);
}

String String::wrap (int width)
{
	std::string lineBreak = "\n";
	std::vector<String> lines = split(lineBreak);

	std::string wrapped;
	for (std::vector<String>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
		if (it != lines.begin())
			wrapped += lineBreak;

		const std::string &line = (*it).str();
		unsigned int index = 0;
		while (index < line.length()) {
			std::string lineSlice(line.substr(index, width));
			wrapped += lineSlice;
			index += width;
			if (index < line.length())
				wrapped += lineBreak;
		}
	}

	return wrapped;
}

bool String::matches (const std::string& text) const
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
