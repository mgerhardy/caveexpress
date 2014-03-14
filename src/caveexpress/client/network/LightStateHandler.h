#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/LightStateMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

class LightStateHandler: public IClientProtocolHandler {
private:
	CaveExpressClientMap& _map;
public:
	LightStateHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const LightStateMessage *msg = static_cast<const LightStateMessage*>(&message);
		const uint16_t id = msg->getEntityId();
		const bool state = msg->getState();
		_map.setCaveState(id, state);
	}
};
