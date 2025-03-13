#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/TimeRemainingMessage.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/windows/IUIMapWindow.h"
#include <string>

class TimeRemainingHandler: public ClientProtocolHandler<TimeRemainingMessage> {
public:
	void execute (const TimeRemainingMessage* msg) override
	{
		const uint16_t secondsRemaining = msg->getSecondsRemaining();
		UI::get().setBarValue(UI_WINDOW_MAP, UINODE_SECONDS_BAR, secondsRemaining);
		
		UINodeLabel* label = UI::get().getNode<UINodeLabel>(UI_WINDOW_MAP, UINODE_SECONDS_REMAINING);
		label->setLabel(string::format("%d:%02d", secondsRemaining / 60, secondsRemaining % 60));
	}
};
