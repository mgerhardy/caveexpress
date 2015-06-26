#pragma once

#include <string>
#include <cstring>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstdarg>
#include <stdio.h>
#include <cstddef>
#include <SDL_platform.h>
#ifdef __ANDROID__
#include <sstream>
#endif

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

class String {
private:

	std::string _string;

public:

	/**
	 * @brief Construct a new empty string.
	 */
	String ();

	virtual ~String ();

	String (const char *string, size_t size);

	/**
	 * @brief Construct a new string from the given nullptr-terminated C string.
	 */
	String (const char *str);

	/**
	 * @brief Construct a new string.
	 */
	String (const std::string &str);

	/**
	 * @brief Construct a copy of the given string.
	 */
	String (const String &str);

	bool startsWith (const std::string& contains) const;

	bool contains (const std::string& contains) const;

	int toInt (int defaultValue = 0) const;

	float toFloat (float defaultValue = 0.0f) const;

	bool toBool (bool defaultValue = false) const;

	String toLower () const;

	String toUpper () const;

	void set (const std::string& string);

	const char *c_str () const;

	bool empty () const;

	std::vector<String> split (const String& delimiters = " \t\r\n\f\v") const;

	inline std::size_t size () const
	{
		return _string.size();
	}

	String substr (std::size_t pos, std::size_t n = std::string::npos) const;

	std::size_t rfind (String string) const;

	/**
	 * Replace all occurrences of @c searchStr with @c replaceStr
	 * @param str source where all occurrences should be replaced
	 * @param searchStr search for this string
	 * @param replaceStr replace with that string
	 * @return string with all occurrences replaced
	 */
	String replaceAll (const String& searchStr, const String& replaceStr) const;

	bool endsWith (const String& end) const;

	String wrap (int width);

	bool matches (const std::string& wildcard) const;

	/**
	 * Returns a new string that has all the spaces from the given string removed.
	 */
	String eraseAllSpaces ();

	String cutAfterFirstMatch (const String& pattern, size_t start = 0);

	String removeFromEnd (const String& pattern);

	/**
	 * @brief Count the number of character (not the number of bytes) of a zero termination string
	 * @note the \\0 termination character is not counted
	 * @note to count the number of bytes, use strlen
	 */
	size_t getUTF8Length () const;

	inline operator const char * () const
	{
		return _string.c_str();
	}

	inline operator const std::string& () const
	{
		return _string;
	}

	inline operator std::string& ()
	{
		return _string;
	}

	inline String & operator+= (const String& string)
	{
		return append(string);
	}

	inline String operator+ (const std::string& string) const
	{
		return String(*this).append(string);
	}

	inline String operator+ (const std::string& string)
	{
		return String(*this).append(string);
	}

	inline String & append (const String& string)
	{
		_string.append(string._string);
		return *this;
	}

	inline operator std::string () const
	{
		return _string;
	}

	const std::string& str () const
	{
		return _string;
	}

	static String format (const char *msg, ...);

	/**
	 * @brief Is this the second or later byte of a multibyte UTF-8 character?
	 * The definition of UTF-8 guarantees that the second and later
	 * bytes of a multibyte character have high bits 10, and that
	 * singlebyte characters and the start of multibyte characters
	 * never do.
	 */
	static bool isUTF8Multibyte (char c);

	/**
	 * @brief length of UTF-8 character starting with this byte.
	 * @return length of character encoding, or 0 if not start of a UTF-8 sequence
	 */
	static size_t getUTF8LengthForCharacter (unsigned char c);

	/**
	 * Calculate how long a Unicode code point would be in UTF-8 encoding.
	 */
	static size_t getUTF8LengthForInt (int c);
};

inline bool operator== (const String& lstr, const String& rstr)
{
	return lstr.str() == rstr.str();
}

inline bool operator== (const String& lstr, const std::string& rstr)
{
	return lstr.str() == rstr;
}

inline bool operator== (const String& lstr, const char* rstr)
{
	return lstr.str() == rstr;
}

inline bool operator!= (const String& lstr, const String& rstr)
{
	return !(lstr == rstr);
}

inline bool operator!= (const String& lstr, const std::string& rstr)
{
	return !(lstr == rstr);
}

inline bool operator!= (const String& lstr, const char* rstr)
{
	return !(lstr == rstr);
}

inline std::string operator+= (const std::string& lstr, const String& rstr)
{
	return lstr + rstr.str();
}

inline std::string operator+ (const std::string& lstr, const String& rstr)
{
	std::string __str(lstr);
	__str.append(rstr.str());
	return __str;
}

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

inline int toInt (const std::string& str)
{
	return atoi(str.c_str());
}

inline bool toBool (const std::string& str)
{
	return str == "1" || str == "true";
}

inline float toFloat (const std::string& str)
{
	return atof(str.c_str());
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
