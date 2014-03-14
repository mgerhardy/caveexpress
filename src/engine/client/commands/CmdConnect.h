#pragma once

#include "engine/common/ICommand.h"
#include "engine/common/Logger.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/ConfigManager.h"

// Connect to a server and trigger the @c LoadMapMessage protocol message
class CmdConnect: public ICommand {
private:
	IClientCallback* _callback;
	ServiceProvider& _serviceProvider;

public:
	CmdConnect (IClientCallback* callback, ServiceProvider& serviceProvider) :
			_callback(callback), _serviceProvider(serviceProvider)
	{
	}

	void run (const Args& args) override
	{
		if (args.size() < 1) {
			error(LOG_NET, "usage: host <port>");
			return;
		}
		const String& host = args[0];
		const int port = args.size() == 2 ? args[1].toInt() : Config.getConfigVar("port")->getIntValue();
		_serviceProvider.getNetwork().openClient(host.c_str(), port, _callback);
	}
};
