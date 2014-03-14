#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/TimeRemainingMessage.h"
#include "engine/client/ui/UI.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"

class TimeRemainingHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const TimeRemainingMessage *msg = static_cast<const TimeRemainingMessage*>(&message);
		const uint16_t secondsRemaining = msg->getSecondsRemaining();
		UI::get().setBarValue(UI_WINDOW_MAP, UINODE_SECONDS_REMAINING, secondsRemaining);
	}
};

