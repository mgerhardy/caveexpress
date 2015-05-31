#pragma once

#include "common/network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/WaterHeightMessage.h"
#include "caveexpress/client/CaveExpressClientMap.h"

class WaterHeightHandler: public IClientProtocolHandler {
private:
	CaveExpressClientMap& _map;
public:
	WaterHeightHandler (CaveExpressClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const WaterHeightMessage *msg = static_cast<const WaterHeightMessage*>(&message);
		_map.setWaterHeight(msg->getHeight());
	}
};

