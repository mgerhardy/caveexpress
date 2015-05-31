#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/BackToMainMessage.h"

class BackToMainHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const BackToMainMessage *msg = static_cast<const BackToMainMessage*>(&message);
		Commands.executeCommandLine(CMD_CL_DISCONNECT);
		UI::get().popMain();
		const std::string& window = msg->getWindow();
		if (window.empty())
			return;
		UI::get().push(window);
	}
};
