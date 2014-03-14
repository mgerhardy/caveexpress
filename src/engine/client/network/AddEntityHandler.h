#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/AddEntityMessage.h"
#include "engine/client/ClientMap.h"

class AddEntityHandler: public IClientProtocolHandler {
protected:
	ClientMap& _map;
public:
	AddEntityHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const AddEntityMessage *msg = static_cast<const AddEntityMessage*>(&message);
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
		ClientEntityPtr entity = ClientEntityRegistry::get(type, id, sprite, animation, xpos, ypos, sizeX, sizeY, angle, spriteAlign);
		if (!entity)
			System.exit("no entity type registered for " + type.name, 1);
		_map.addEntity(entity);
	}
};
