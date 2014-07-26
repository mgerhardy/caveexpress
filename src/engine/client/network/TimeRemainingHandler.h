#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/TimeRemainingMessage.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/windows/IUIMapWindow.h"

class TimeRemainingHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const TimeRemainingMessage *msg = static_cast<const TimeRemainingMessage*>(&message);
		const uint16_t secondsRemaining = msg->getSecondsRemaining();
		UI::get().setBarValue(UI_WINDOW_MAP, UINODE_SECONDS_REMAINING, secondsRemaining);
	}
};

