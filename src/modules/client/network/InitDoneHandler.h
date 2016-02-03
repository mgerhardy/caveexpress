#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/InitDoneMessage.h"
#include "client/ClientMap.h"

class InitDoneHandler: public ClientProtocolHandler<InitDoneMessage> {
protected:
	ClientMap& _map;
public:
	InitDoneHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const InitDoneMessage* msg) override
	{
		// TODO: close console?
		const uint16_t id = msg->getPlayerId();
		_map.init(id);
	}
};
