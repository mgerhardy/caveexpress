#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/WaterHeightMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

namespace caveexpress {

class WaterHeightHandler: public ClientProtocolHandler<WaterHeightMessage> {
private:
	CaveExpressClientMap& _map;
public:
	WaterHeightHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const WaterHeightMessage* msg) override
	{
		_map.setWaterHeight(msg->getHeight());
	}
};

}
