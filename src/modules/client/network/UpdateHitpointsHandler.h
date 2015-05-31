#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdateHitpointsMessage.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/windows/IUIMapWindow.h"

class UpdateHitpointsHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const UpdateHitpointsMessage *msg = static_cast<const UpdateHitpointsMessage*>(&message);
		UINodeBar* bar = UI::get().setBarValue(UI_WINDOW_MAP, UINODE_HITPOINTS, msg->getHitpoints());
		bar->flash();
	}
};
