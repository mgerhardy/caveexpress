#pragma once

#include "engine/client/entities/ClientEntity.h"

class ClientParticle: public ClientEntity {
private:
	struct ParticleData {
		ParticleData() :
				angle(0), lifetime(0) {
		}

		EntityAngle angle;
		uint32_t lifetime;
		vec2 pos;
		vec2 prevPos;
		vec2 nextPos;
	};

	ParticleData *_particles;
	int _maxParticles;
	int _lifetime;
	std::string _sprite;

	ClientParticle (uint16_t id, const std::string& sprite, float x, float y);
public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const;
	};
	static Factory FACTORY;

	virtual ~ClientParticle ();

	void resetParticles (uint8_t maxParticles, uint32_t lifetime);
	void updateParticle (int index, float x, float y, uint32_t lifetime, EntityAngle angle);
	void setSprite (const std::string& sprite);

	// ClientEntity
	bool update (uint32_t deltaTime, bool lerpPos);
	void render (IFrontend *frontend, Layer layer, int scale, int offsetX, int offsetY) const override;
};

inline void ClientParticle::setSprite (const std::string& sprite)
{
	_sprite = sprite;
}
