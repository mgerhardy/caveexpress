#pragma once

#include "engine/common/MapSettings.h"
#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/MapSettingsMessage.h"

class MapSettingsHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const MapSettingsMessage *msg = static_cast<const MapSettingsMessage*>(&message);
		const std::map<std::string, std::string>& settings = msg->getSettings();
		for (std::map<std::string, std::string>::const_iterator i = settings.begin(); i != settings.end(); ++i) {
			info(LOG_CLIENT, i->first + ": " + i->second);
		}
	}
};
