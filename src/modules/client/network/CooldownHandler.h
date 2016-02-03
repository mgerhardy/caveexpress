#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/CooldownMessage.h"
#include "client/ClientMap.h"

class CooldownHandler: public ClientProtocolHandler<CooldownMessage> {
private:
	ClientMap& _map;
public:
	CooldownHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const CooldownMessage* msg) override
	{
		const Cooldown& cooldown = msg->getCooldown();
		_map.cooldown(cooldown);
	}
};
