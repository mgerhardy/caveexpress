#pragma once

#include "engine/common/ICommand.h"
#include "engine/common/Logger.h"
#include "engine/common/network/INetwork.h"

class CmdDisconnect: public ICommand {
private:
	ServiceProvider& _serviceProvider;

public:
	CmdDisconnect (ServiceProvider& serviceProvider) :
			_serviceProvider(serviceProvider)
	{
	}

	void run (const Args& args) override
	{
		INetwork& network = _serviceProvider.getNetwork();
		network.closeClient();
		if (network.isServer()) {
			// TODO: the map (server side) is still running
			network.closeServer();
		}
	}
};
