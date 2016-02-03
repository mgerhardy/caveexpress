#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/PauseMessage.h"
#include "client/ClientMap.h"

class PauseHandler: public ClientProtocolHandler<PauseMessage> {
private:
	ClientMap& _map;
public:
	PauseHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const PauseMessage* msg) override
	{
		const bool pause = msg->isPause();
		_map.setPause(pause);
	}
};

