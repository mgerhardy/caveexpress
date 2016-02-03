#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdateHitpointsMessage.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/windows/IUIMapWindow.h"

class UpdateHitpointsHandler: public ClientProtocolHandler<UpdateHitpointsMessage> {
public:
	void execute (const UpdateHitpointsMessage* msg) override
	{
		UINodeBar* bar = UI::get().setBarValue(UI_WINDOW_MAP, UINODE_HITPOINTS, msg->getHitpoints());
		bar->flash();
	}
};
