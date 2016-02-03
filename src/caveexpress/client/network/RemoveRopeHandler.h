#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/shared/network/messages/RemoveRopeMessage.h"
#include "client/ClientMap.h"

namespace caveexpress {

class RemoveRopeHandler: public ClientProtocolHandler<RemoveRopeMessage> {
private:
	ClientMap& _map;
public:
	RemoveRopeHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const RemoveRopeMessage* msg) override
	{
		const uint16_t id1 = msg->getEntityId();
		ClientEntityPtr entity = _map.getEntity(id1);
		if (!entity)
			return;

		entity->removeRope();
	}
};

}
