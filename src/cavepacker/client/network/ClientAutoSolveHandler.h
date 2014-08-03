#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"
#include "engine/client/ui/UI.h"

class ClientAutoSolveHandler: public IClientProtocolHandler {
private:
	bool _started;
public:
	ClientAutoSolveHandler(bool started) :_started(started) {
	}

	void execute (const IProtocolMessage& message) override
	{
		UIMapWindow* window = static_cast<UIMapWindow*>(UI::get().getWindow(UI_WINDOW_MAP));
		if (_started) {
			window->hideHud();
			window->showAutoSolveSlider();
		} else {
			window->showHud();
			window->hideAutoSolveSlider();
		}
	}
};
