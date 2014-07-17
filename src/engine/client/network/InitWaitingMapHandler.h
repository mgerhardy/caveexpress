#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/client/ui/windows/IUIMapWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/common/ServiceProvider.h"

/**
 * @brief The server is done loading everything and now waits for the client to start the session.
 */
class InitWaitingMapHandler: public IClientProtocolHandler {
private:
	ServiceProvider& _serviceProvider;
public:
	InitWaitingMapHandler (ServiceProvider& serviceProvider) :
			_serviceProvider(serviceProvider)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		IUIMapWindow* window = static_cast<IUIMapWindow*>(UI::get().getWindow(UI_WINDOW_MAP));
		window->initWaitingForPlayers(_serviceProvider.getNetwork().isServer());
	}
};
