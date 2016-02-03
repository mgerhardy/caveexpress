#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/AnnounceTargetCaveMessage.h"
#include "caveexpress/client/entities/ClientNPC.h"
#include "client/ClientMap.h"
#include "client/ClientMap.h"

namespace caveexpress {

class AnnounceTargetCaveHandler: public ClientProtocolHandler<AnnounceTargetCaveMessage> {
private:
	CaveExpressClientMap& _map;
public:
	AnnounceTargetCaveHandler(CaveExpressClientMap& map) :
			_map(map) {
	}

	void execute(const AnnounceTargetCaveMessage* msg) override
	{
		const uint16_t id = msg->getNpcId();
		const uint16_t delay = msg->getDelay();
		const uint8_t caveNumber = msg->getCaveNumber();
		ClientEntityPtr entity = _map.getEntity(id);
		if (entity && EntityTypes::isNpcCave(entity->getType())) {
			ClientNPC* npc = static_cast<ClientNPC*>(entity);
			npc->setTargetCave(caveNumber, delay);
		}
	}
};

}
