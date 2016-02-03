#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdateParticleMessage.h"
#include "client/ClientMap.h"
#include "caveexpress/client/entities/ClientParticle.h"

namespace caveexpress {

class UpdateParticleHandler: public ClientProtocolHandler<UpdateParticleMessage> {
private:
	ClientMap& _map;
public:
	UpdateParticleHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const UpdateParticleMessage* msg) override
	{
		const uint16_t id = msg->getEntityId();
		ClientEntityPtr entity = _map.getEntity(id);
		if (!entity) {
			entity = ClientEntityRegistry::get(EntityTypes::PARTICLE, id);
			if (!entity)
				System.exit("no entity type registered for " + EntityTypes::PARTICLE.name, 1);
			_map.addEntity(entity);
		}

		ClientParticle *particle = static_cast<ClientParticle*>(entity);
		particle->resetParticles(msg->getMaxParticles(), msg->getMaxLifeTime());
		const std::string& sprite = msg->getSprite();
		particle->setSprite(sprite);
		const uint8_t bodyCount = msg->getBodyCount();
		for (int i = 0; i < bodyCount; i++) {
			const float x = msg->getX(i);
			const float y = msg->getY(i);
			const uint32_t lifetime = msg->getLifeTime(i);
			const uint8_t index = msg->getIndex(i);
			const EntityAngle angle = msg->getAngle(i);
			particle->updateParticle(index, x, y, lifetime, angle);
		}
	}
};

}
