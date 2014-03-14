#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/common/ServiceProvider.h"

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
		UIMapWindow* window = static_cast<UIMapWindow*>(UI::get().getWindow(UI_WINDOW_MAP));
		window->initWaitingForPlayers(_serviceProvider.getNetwork().isServer());
	}
};
