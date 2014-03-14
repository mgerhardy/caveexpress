#pragma once

#include <string>

class URI {
private:
	std::string _protocol;
	std::string _host;
	std::string _port;
	std::string _path;
	std::string _query;

public:
	explicit URI (const std::string& uri);
	virtual ~URI ();

	inline const std::string& getProtocol () const
	{
		return _protocol;
	}

	inline const std::string& getHost () const
	{
		return _host;
	}

	inline const std::string& getPort () const
	{
		return _port;
	}

	inline const std::string& getPath () const
	{
		return _path;
	}

	inline const std::string& getQuery () const
	{
		return _query;
	}

	inline std::string print () const
	{
		return _protocol + "://" + _host + (_port.empty() ? "" : ":" + _port) + _path + _query;
	}
};
