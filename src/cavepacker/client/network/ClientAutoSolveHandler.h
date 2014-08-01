#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "cavepacker/shared/network/messages/AutoSolveStartedMessage.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"

class ClientAutoSolveHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		UIMapWindow* window = static_cast<UIMapWindow*>(UI::get().getWindow(UI_WINDOW_MAP));
		window->hideHud();
		window->showAutoSolveSlider();
	}
};
