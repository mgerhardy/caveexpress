#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/PauseMessage.h"
#include "engine/client/ClientMap.h"

class PauseHandler: public IClientProtocolHandler {
private:
	ClientMap& _map;
public:
	PauseHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const PauseMessage *msg = static_cast<const PauseMessage*>(&message);
		const bool pause = msg->isPause();
		_map.setPause(pause);
	}
};

