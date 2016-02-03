#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/RemoveEntityMessage.h"
#include "client/ClientMap.h"

class RemoveEntityHandler: public ClientProtocolHandler<RemoveEntityMessage> {
private:
	ClientMap& _map;
public:
	RemoveEntityHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const RemoveEntityMessage* msg) override
	{
		const uint16_t id = msg->getEntityId();
		const bool fadeOut = msg->isFadeOut();
		_map.removeEntity(id, fadeOut);
	}
};
