#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/MapSettingsMessage.h"

class MapSettingsHandler: public IClientProtocolHandler {
protected:
	ClientMap& _map;
public:
	MapSettingsHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const MapSettingsMessage *msg = static_cast<const MapSettingsMessage*>(&message);
		const std::map<std::string, std::string>& settings = msg->getSettings();
		for (std::map<std::string, std::string>::const_iterator i = settings.begin(); i != settings.end(); ++i) {
			_map.setSetting(i->first, i->second);
		}
	}
};
