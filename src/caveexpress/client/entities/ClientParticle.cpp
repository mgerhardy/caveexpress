#include "ClientParticle.h"
#include "sound/Sound.h"
#include "common/SoundType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "ui/UI.h"
#include "common/Log.h"
#include "common/Shared.h"

namespace caveexpress {

ClientParticle::ClientParticle (uint16_t id, const std::string& sprite, float x, float y) :
		ClientEntity(EntityTypes::PARTICLE, id, x, y, 0.01f, 0.01f,
				SoundMapping(), ENTITY_ALIGN_MIDDLE_CENTER, 0), _particles(
				nullptr), _maxParticles(0), _lifetime(0) {
}

ClientParticle::~ClientParticle ()
{
	resetParticles(0, 0);
}

void ClientParticle::resetParticles (uint8_t maxParticles, uint32_t lifetime)
{
	_lifetime = lifetime;
	if (_maxParticles == maxParticles)
		return;

	delete [] _particles;
	_particles = nullptr;
	_maxParticles = maxParticles;
	if (_maxParticles <= 0)
		return;

	_particles = new ParticleData[maxParticles];
}

void ClientParticle::updateParticle (int index, float x, float y, uint32_t lifetime, EntityAngle angle)
{
	if (index >= _maxParticles || index < 0) {
		Log::error(LOG_CLIENT, "invalid particle index given");
		return;
	}
	ParticleData& d = _particles[index];
	d.pos = vec2(x, y);
	d.prevPos = d.nextPos;
	d.nextPos = d.pos;
	d.angle = angle;
	d.lifetime = lifetime;
}

void ClientParticle::render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY) const
{
	const TexturePtr& texture = UI::get().loadTexture(_sprite);
	if (!texture || !texture->isValid()) {
		Log::error(LOG_CLIENT, "client particle texture '%s' not found", _sprite.c_str());
		return;
	}

	const int basePosX = offsetX - texture->getWidth() / 2;
	const int basePosY = offsetY - texture->getHeight() / 2;
	for (int i = 0; i < _maxParticles; ++i) {
		ParticleData &p = _particles[i];
		if (p.lifetime <= 0)
			continue;
		const int posX = basePosX + p.pos.x * scale;
		const int posY = basePosY + p.pos.y * scale;
		const float alpha = p.lifetime / static_cast<float>(_lifetime);
		frontend->renderImage(texture.get(), posX, posY, 20.0f * zoom, 16.0f * zoom, p.angle, alpha);
	}
}

bool ClientParticle::update (uint32_t deltaTime, bool lerpPos)
{
	for (int i = 0; i < _maxParticles; ++i) {
		ParticleData &p = _particles[i];
		if (p.lifetime <= 0)
			continue;
		if (p.lifetime < deltaTime)
			p.lifetime = 0;
		else
			p.lifetime -= deltaTime;
	}
	return ClientEntity::update(deltaTime, lerpPos);
}

ClientEntityPtr ClientParticle::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	return ClientEntityPtr(
			new ClientParticle(ctx->id, ctx->sprite, ctx->x, ctx->y));
}

ClientParticle::Factory ClientParticle::FACTORY;

}
