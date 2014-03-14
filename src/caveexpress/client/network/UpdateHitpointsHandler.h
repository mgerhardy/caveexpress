#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/UpdateHitpointsMessage.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"

class UpdateHitpointsHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const UpdateHitpointsMessage *msg = static_cast<const UpdateHitpointsMessage*>(&message);
		UINodeBar* bar = UI::get().setBarValue(UI_WINDOW_MAP, UINODE_HITPOINTS, msg->getHitpoints());
		bar->flash();
	}
};
