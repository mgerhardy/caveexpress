#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/LightStateMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

namespace caveexpress {

class LightStateHandler: public ClientProtocolHandler<LightStateMessage> {
private:
	CaveExpressClientMap& _map;
public:
	LightStateHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const LightStateMessage* msg) override
	{
		const uint16_t id = msg->getEntityId();
		const bool state = msg->getState();
		_map.setCaveState(id, state);
	}
};

}
