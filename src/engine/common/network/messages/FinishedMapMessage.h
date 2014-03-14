#pragma once

#include "engine/common/network/IProtocolMessage.h"
#include <string>

class FinishedMapMessage: public IProtocolMessage {
private:
	std::string _mapName;
	uint16_t _finishPoints;
	uint32_t _time;
	uint8_t _stars;

public:
	FinishedMapMessage (const std::string& mapName, uint32_t finishPoints, uint32_t time, uint8_t stars) :
			IProtocolMessage(protocol::PROTO_FINISHEDMAP), _mapName(mapName), _finishPoints(finishPoints), _time(time), _stars(
					stars)
	{
	}

	FinishedMapMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_FINISHEDMAP)
	{
		_mapName = input.readString();
		_finishPoints = input.readShort();
		_time = input.readInt();
		_stars = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_mapName);
		out.addShort(_finishPoints);
		out.addInt(_time);
		out.addByte(_stars);
	}

	inline const std::string& getMapName () const
	{
		return _mapName;
	}

	inline uint16_t getFinishPoints () const
	{
		return _finishPoints;
	}

	inline uint32_t getTime () const
	{
		return _time;
	}

	inline uint8_t getStars () const
	{
		return _stars;
	}
};
