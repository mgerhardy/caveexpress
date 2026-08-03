#pragma once

#include "client/entities/ClientEntity.h"

class ClientMapTile: public ClientEntity {
protected:
	std::string _sprite;

	ClientMapTile (const EntityType& type, uint16_t id, const std::string& sprite, const Animation& animation, float x,
			float y, float sizeX, float sizeY, EntityAngle angle, const SoundMapping& soundMapping, EntityAlignment align);
public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const override;
	};
	static Factory FACTORY;

	virtual ~ClientMapTile ();

	/**
	 * @brief Access to the initial sprite name (might e.g. be changed if you change the animation)
	 */
	std::string getSpriteName () const override { return _sprite; }

	/**
	 * @brief Set a new sprite - but does not change the original sprite name
	 */
	void setNewSprite (const std::string& spriteName);

	// ClientEntity
	bool update (uint32_t deltaTime, bool lerpPos, bool animateSpriteAlways = true) override;
};
