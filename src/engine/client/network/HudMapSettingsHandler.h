#pragma once

#include "engine/common/MapSettings.h"
#include "engine/client/network/MapSettingsHandler.h"
#include "engine/client/ClientMap.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/windows/IUIMapWindow.h"
#include "engine/client/ui/nodes/IUINodeMap.h"

class HudMapSettingsHandler: public MapSettingsHandler {
public:
	HudMapSettingsHandler (ClientMap& map) :
			MapSettingsHandler(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		MapSettingsHandler::execute(message);
		const MapSettingsMessage *msg = static_cast<const MapSettingsMessage*>(&message);
		const std::map<std::string, std::string>& settings = msg->getSettings();
		std::map<std::string, std::string>::const_iterator i = settings.find(msn::REFERENCETIME);
		uint16_t seconds;
		if (i == settings.end()) {
			seconds = msdv::REFERENCETIME;
		} else {
			seconds = string::toInt(i->second);
		}
		UI::get().setBarValue(UI_WINDOW_MAP, UINODE_SECONDS_REMAINING, seconds);
		UI::get().setBarMax(UI_WINDOW_MAP, UINODE_SECONDS_REMAINING, seconds);
	}
};
