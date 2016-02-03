#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/shared/network/messages/AddRopeMessage.h"
#include "client/ClientMap.h"

namespace caveexpress {

class AddRopeHandler: public ClientProtocolHandler<AddRopeMessage> {
private:
	ClientMap& _map;
public:
	AddRopeHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const AddRopeMessage* msg) override
	{
		const uint16_t id1 = msg->getEntityId1();
		const uint16_t id2 = msg->getEntityId2();
		ClientEntityPtr entity1 = _map.getEntity(id1);
		ClientEntityPtr entity2 = _map.getEntity(id2);
		if (entity1 && entity2)
			entity1->addRope(entity2);
	}
};

}
