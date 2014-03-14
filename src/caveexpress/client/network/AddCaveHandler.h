#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/AddCaveMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

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
		const bool state = msg->getState();
		_map.setCaveState(id, state);
	}
};
