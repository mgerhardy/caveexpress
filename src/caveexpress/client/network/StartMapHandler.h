#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "engine/client/ui/UI.h"

/**
 * @brief Informs the client that the map is now started
 */
class StartMapHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		UIMapWindow* window = static_cast<UIMapWindow*>(UI::get().getWindow(UI_WINDOW_MAP));
		window->start();
	}
};
