#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/InitDoneMessage.h"
#include "engine/client/ClientMap.h"

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
