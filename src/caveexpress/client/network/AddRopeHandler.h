#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/shared/network/messages/AddRopeMessage.h"
#include "client/ClientMap.h"

class AddRopeHandler: public IClientProtocolHandler {
private:
	ClientMap& _map;
public:
	AddRopeHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const AddRopeMessage *msg = static_cast<const AddRopeMessage*>(&message);
		const uint16_t id1 = msg->getEntityId1();
		const uint16_t id2 = msg->getEntityId2();
		ClientEntityPtr entity1 = _map.getEntity(id1);
		ClientEntityPtr entity2 = _map.getEntity(id2);
		if (entity1 && entity2)
			entity1->addRope(entity2);
	}
};
