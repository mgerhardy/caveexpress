#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/GateStateMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

namespace caveexpress {

class GateStateHandler: public ClientProtocolHandler<GateStateMessage> {
private:
	CaveExpressClientMap& _map;
public:
	GateStateHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const GateStateMessage* msg) override
	{
		_map.setGateState(msg->getEntityId(), msg->getOpenAmount());
	}
};

}
