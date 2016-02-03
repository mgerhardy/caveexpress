#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/WaterImpactMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

namespace caveexpress {

class WaterImpactHandler: public ClientProtocolHandler<WaterImpactMessage> {
private:
	CaveExpressClientMap& _map;
public:
	WaterImpactHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const WaterImpactMessage* msg) override
	{
		const float x = msg->getImpactX();
		const float force = msg->getImpactForce();
		_map.handleWaterImpact(x, force);
	}
};

}
