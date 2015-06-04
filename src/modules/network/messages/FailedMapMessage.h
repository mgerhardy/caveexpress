#pragma once

#include "network/IProtocolMessage.h"
#include "common/MapFailedReason.h"
#include "common/ThemeType.h"

class FailedMapMessage: public IProtocolMessage {
private:
	std::string _mapName;
	const MapFailedReason* _reason;
	const ThemeType* _theme;

public:
	FailedMapMessage (const std::string& mapName, const MapFailedReason& reason, const ThemeType& theme) :
			IProtocolMessage(protocol::PROTO_FAILEDMAP), _mapName(mapName), _reason(&reason), _theme(&theme)
	{
	}

	PROTOCOL_CLASS_FACTORY(FailedMapMessage);

	explicit FailedMapMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_FAILEDMAP)
	{
		_mapName = input.readString();
		_reason = &MapFailedReason::get(input.readByte());
		_theme = &ThemeType::get(input.readByte());
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addString(_mapName);
		out.addByte(_reason->id);
		out.addByte(_theme->id);
	}

	inline const std::string& getMapName () const
	{
		return _mapName;
	}

	inline const MapFailedReason& getReason () const
	{
		return *_reason;
	}

	inline const ThemeType& getTheme () const
	{
		return *_theme;
	}
};
