#pragma once

#include "common/Math.h"
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
		const bool red = msg->getHitpoints() < 30;
		const bool yellow = msg->getHitpoints() < 60;
		bar->setBarColor(   red ? colorRed : yellow ? colorYellow : colorGreen);
		bar->setBorderColor(red ? colorRed : yellow ? colorYellow : colorGreen);
		bar->flash(2000, red ? 2 : 1);
	}
};
