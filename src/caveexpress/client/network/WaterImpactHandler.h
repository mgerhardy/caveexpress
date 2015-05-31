#pragma once

#include "common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/WaterImpactMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

class WaterImpactHandler: public IClientProtocolHandler {
private:
	CaveExpressClientMap& _map;
public:
	WaterImpactHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const WaterImpactMessage *msg = static_cast<const WaterImpactMessage*>(&message);
		const float x = msg->getImpactX();
		const float force = msg->getImpactForce();
		_map.handleWaterImpact(x, force);
	}
};

