#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/TimeRemainingMessage.h"
#include "client/ui/UI.h"
#include "client/ui/windows/IUIMapWindow.h"

class TimeRemainingHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const TimeRemainingMessage *msg = static_cast<const TimeRemainingMessage*>(&message);
		const uint16_t secondsRemaining = msg->getSecondsRemaining();
		UI::get().setBarValue(UI_WINDOW_MAP, UINODE_SECONDS_REMAINING, secondsRemaining);
	}
};

