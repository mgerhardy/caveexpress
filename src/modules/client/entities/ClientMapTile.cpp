#include "client/entities/ClientMapTile.h"
#include "ui/UI.h"
#include "client/sound/Sound.h"

ClientMapTile::ClientMapTile (const EntityType& type, uint16_t id, const std::string& sprite,
		const Animation& animation, float x, float y, float sizeX, float sizeY, EntityAngle angle,
		const SoundMapping& soundMapping, EntityAlignment align) :
		ClientEntity(type, id, x, y, sizeX, sizeY, soundMapping, align, angle)
{
	_currSprite = UI::get().loadSprite(sprite);
	if (_currSprite->getFrameCount() > 1)
		_currSprite = SpritePtr(_currSprite->copy());
	setScreenSize(_currSprite->getMaxWidth(), _currSprite->getMaxHeight());
}

ClientMapTile::~ClientMapTile ()
{
}

bool ClientMapTile::update (uint32_t deltaTime, bool lerpPos)
{
	ClientEntity::update(deltaTime, lerpPos);
	if (_currSprite)
		_currSprite->update(deltaTime);

	return true;
}

ClientEntityPtr ClientMapTile::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	return ClientEntityPtr(
			new ClientMapTile(ctx->type, ctx->id, ctx->sprite, ctx->animation, ctx->x, ctx->y, ctx->width, ctx->height, ctx->angle, ctx->soundMapping, ctx->align));
}

ClientMapTile::Factory ClientMapTile::FACTORY;
