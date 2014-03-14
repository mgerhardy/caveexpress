#pragma once

#include "engine/common/ICommand.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/network/messages/PingMessage.h"
#include "engine/client/ui/nodes/UINodeServerSelector.h"

class CmdServerPing: public ICommand, public IClientCallback {
private:
	UINodeServerSelector *_serverSelector;
	ServiceProvider& _serviceProvider;

public:
	CmdServerPing (UINodeServerSelector *serverlist, ServiceProvider& serviceProvider) :
			_serverSelector(serverlist), _serviceProvider(serviceProvider)
	{
	}

	void run (const Args& args) override
	{
		_serverSelector->reset();
		if (!_serviceProvider.getNetwork().discover(this, Config.getPort()))  {
			error(LOG_CLIENT, "could not send the ping broadcast");
		} else {
			info(LOG_CLIENT, "sent ping broadcast");
		}
	}

	void onOOBData (const std::string& host, const IProtocolMessage* message) override
	{
		if (message->getId() != protocol::PROTO_PING) {
			error(LOG_CLIENT, "got invalid discover broadcast reply");
			return;
		}
		info(LOG_CLIENT, "got ping broadcast reply");
		const PingMessage* p = static_cast<const PingMessage*>(message);
		_serverSelector->addServer(host, p->getName(), p->getMapName(), p->getPort(), p->getPlayerCount(),
				p->getMaxPlayerCount());
	}
};
