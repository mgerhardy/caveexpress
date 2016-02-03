#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/AddEntityMessage.h"
#include "client/ClientMap.h"
#include "common/System.h"

class AddEntityHandler: public ClientProtocolHandler<AddEntityMessage> {
protected:
	ClientMap& _map;
public:
	AddEntityHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const AddEntityMessage* msg) override
	{
		const uint16_t id = msg->getEntityId();
		const Animation& animation = msg->getAnimation();
		const EntityType& type = msg->getEntityType();
		const std::string& sprite = msg->getSprite();
		const float xpos = msg->getX();
		const float ypos = msg->getY();
		const float sizeX = msg->getWidth();
		const float sizeY = msg->getHeight();
		const EntityAngle angle = msg->getAngle();
		const EntityAlignment spriteAlign = msg->getSpriteAlignment();
		const ClientEntityPtr& entity = ClientEntityRegistry::get(type, id, sprite, animation, xpos, ypos, sizeX, sizeY, angle, spriteAlign);
		if (!entity)
			System.exit("no entity type registered for " + type.name, 1);
		_map.addEntity(entity);
	}
};
