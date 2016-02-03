#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/TimeRemainingMessage.h"
#include "ui/UI.h"
#include "ui/windows/IUIMapWindow.h"

class TimeRemainingHandler: public ClientProtocolHandler<TimeRemainingMessage> {
public:
	void execute (const TimeRemainingMessage* msg) override
	{
		const uint16_t secondsRemaining = msg->getSecondsRemaining();
		UI::get().setBarValue(UI_WINDOW_MAP, UINODE_SECONDS_REMAINING, secondsRemaining);
	}
};

