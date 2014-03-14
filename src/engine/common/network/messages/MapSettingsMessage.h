#pragma once

#include "engine/common/network/IProtocolMessage.h"
#include <string>
#include <map>

class MapSettingsMessage: public IProtocolMessage {
private:
	typedef std::map<std::string, std::string> Map;
	Map _settings;
	const Map* _settingsPtr;
public:
	MapSettingsMessage (const std::map<std::string, std::string>& settings) :
			IProtocolMessage(protocol::PROTO_MAPSETTINGS), _settingsPtr(&settings)
	{
	}

	MapSettingsMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_MAPSETTINGS), _settingsPtr(nullptr)
	{
		const int16_t size = input.readShort();
		for (int16_t i = 0; i < size; ++i) {
			const std::string key = input.readString();
			const std::string value = input.readString();
			_settings[key] = value;
		}
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_settingsPtr->size());
		for (Map::const_iterator i = _settingsPtr->begin(); i != _settingsPtr->end(); ++i) {
			out.addString(i->first);
			out.addString(i->second);
		}
	}

	inline const std::map<std::string, std::string>& getSettings () const
	{
		return _settings;
	}
};
