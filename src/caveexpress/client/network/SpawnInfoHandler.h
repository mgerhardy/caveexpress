#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/SpawnInfoMessage.h"
#include "client/ClientMap.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "ui/UI.h"

namespace caveexpress {

class SpawnInfoHandler: public ClientProtocolHandler<SpawnInfoMessage> {
private:
	ClientMap& _clientMap;
	IUINodeMap *_mapNode;
public:
	SpawnInfoHandler(ClientMap& clientMap, IUINodeMap* mapNode) :
			_clientMap(clientMap), _mapNode(mapNode) {
	}

	void execute(const SpawnInfoMessage* msg) override
	{
		const EntityType& entityType = msg->getEntityType();
		const float xpos = msg->getX();
		const float ypos = msg->getY();
		const vec2 position(xpos, ypos);
		_clientMap.spawnInfo(position, entityType);

		if (!_clientMap.wantInformation(entityType))
			return;

		if (EntityTypes::isNpcFlying(entityType)) {
			_mapNode->displayText(tr("Pterodactyls is after you"));
		} else if (EntityTypes::isNpcFish(entityType)) {
			_mapNode->displayText(tr("A mighty fish is after you"));
		}
	}
};

}
