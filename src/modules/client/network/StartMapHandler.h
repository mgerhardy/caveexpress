#pragma once

#include "network/IProtocolHandler.h"
#include "ui/windows/IUIMapWindow.h"
#include "ui/UI.h"

/**
 * @brief Informs the client that the map is now started
 */
class StartClientMapHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		IUIMapWindow* window = static_cast<IUIMapWindow*>(UI::get().getWindow(UI_WINDOW_MAP));
		window->start();
	}
};
