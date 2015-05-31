#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/InitDoneMessage.h"
#include "client/ClientMap.h"

class InitDoneHandler: public IClientProtocolHandler {
protected:
	ClientMap& _map;
public:
	InitDoneHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const InitDoneMessage *msg = static_cast<const InitDoneMessage*>(&message);
		// TODO: close console?
		const uint16_t id = msg->getPlayerId();
		_map.init(id);
	}
};
