#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/LoadMapMessage.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "engine/common/Logger.h"

class LoadMapHandler: public IClientProtocolHandler {
private:
	ServiceProvider& _serviceProvider;

public:
	LoadMapHandler (ServiceProvider& serviceProvider) :
			_serviceProvider(serviceProvider)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const SpawnMessage spawnMsg;
		if (_serviceProvider.getNetwork().sendToServer(spawnMsg) == -1) {
			error(LOG_CLIENT, "could not send spawn command to server");
			return;
		}

		const LoadMapMessage *msg = static_cast<const LoadMapMessage*>(&message);
		info(LOG_CLIENT, "load map: " + msg->getName());
	}
};
