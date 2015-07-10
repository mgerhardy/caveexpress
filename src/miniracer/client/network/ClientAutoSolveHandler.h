#pragma once

#include "network/IProtocolHandler.h"
#include "miniracer/shared/network/messages/ProtocolMessages.h"
#include "miniracer/client/ui/windows/UIMapWindow.h"
#include "ui/UI.h"

namespace miniracer {

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

}
