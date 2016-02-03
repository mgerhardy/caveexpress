#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/BackToMainMessage.h"

class BackToMainHandler: public ClientProtocolHandler<BackToMainMessage> {
public:
	void execute (const BackToMainMessage* msg) override
	{
		Commands.executeCommandLine(CMD_CL_DISCONNECT);
		UI::get().popMain();
		const std::string& window = msg->getWindow();
		if (window.empty())
			return;
		UI::get().push(window);
	}
};
