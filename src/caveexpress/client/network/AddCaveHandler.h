#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/AddCaveMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

namespace caveexpress {

class AddCaveHandler: public IClientProtocolHandler {
private:
	CaveExpressClientMap& _map;
public:
	AddCaveHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const AddCaveMessage *msg = static_cast<const AddCaveMessage*>(&message);
		const uint16_t id = msg->getEntityId();
		const uint8_t number = msg->getCaveNumber();
		const bool state = msg->getState();
		_map.setCaveNumber(id, number);
		_map.setCaveState(id, state);
	}
};

}
