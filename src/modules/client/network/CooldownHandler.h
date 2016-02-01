#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/CooldownMessage.h"
#include "client/ClientMap.h"

class CooldownHandler: public IClientProtocolHandler {
private:
	ClientMap& _map;
public:
	CooldownHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const CooldownMessage *msg = static_cast<const CooldownMessage*>(&message);
		const Cooldown& cooldown = msg->getCooldown();
		_map.cooldown(cooldown);
	}
};
