#pragma once

#include <iostream>
#include <sstream>

class ParameterList {
private:
	std::ostringstream s;
	const std::string _sep;
	const std::string _begin;
	const std::string _end;
public:
	ParameterList (const std::string& sep = ",", const std::string& begin = "", const std::string &end = "") :
			_sep(sep), _begin(begin), _end(end)
	{
	}

	void add (const std::string& string)
	{
		if (!s.str().empty()) {
			s << _sep;
		}
		s << string;
	}

	std::string str ()
	{
		return _begin + s.str() + _end;
	}
};
