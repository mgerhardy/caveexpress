#pragma once

#include "engine/common/network/IProtocolMessage.h"
#include <string>

// inform the client that the map should get loaded. After the client loaded the map, it
// will send a spawn command to the server which will trigger the player spawn on the map
// @sa Map::initPlayer
class LoadMapMessage: public IProtocolMessage {
private:
	std::string _name;
	std::string _title;

public:
	LoadMapMessage (const std::string& name, const std::string& title) :
			IProtocolMessage(protocol::PROTO_LOADMAP), _name(name), _title(title)
	{
	}

	LoadMapMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_LOADMAP)
	{
		_name = input.readString();
		_title = input.readString();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_name);
		out.addString(_title);
	}

	inline const std::string& getName () const
	{
		return _name;
	}

	inline const std::string& getTitle () const
	{
		return _title;
	}
};
