#pragma once

#include "engine/client/entities/ClientEntity.h"

class ClientMapTile: public ClientEntity {
protected:
	ClientMapTile (const EntityType& type, uint16_t id, const std::string& sprite, const Animation& animation, float x,
			float y, float sizeX, float sizeY, EntityAngle angle, const SoundMapping& soundMapping, EntityAlignment align);
public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const;
	};
	static Factory FACTORY;

	virtual ~ClientMapTile ();

	// ClientEntity
	bool update (uint32_t deltaTime, bool lerpPos) override;
};
