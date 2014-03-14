#include "ClientCaveTile.h"
#include "engine/client/sound/Sound.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"

ClientCaveTile::ClientCaveTile (uint16_t id, const std::string& sprite, const Animation& animation, float x,
		float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align) :
		ClientMapTile(EntityTypes::CAVE, id, sprite, animation, x, y, sizeX, sizeY, 0, soundMapping, align), _sprite(sprite), _lightState(
				DEFAULT_LIGHT_STATE)
{
	const SoundType& soundType = SoundTypes::SOUND_AMBIENT_CAVE;
	_animationSound = SoundControl.play(soundType.getSound(), getPos(), soundType.isLoopSound());
}

ClientCaveTile::~ClientCaveTile ()
{
}

void ClientCaveTile::render (IFrontend *frontend, Layer layer, int scale, int offsetX, int offsetY) const
{
	if (_currSprite) {
		if (_lightState)
			_currSprite->setCurrentFrame(0);
		else
			_currSprite->setCurrentFrame(1);
	}

	ClientMapTile::render(frontend, layer, scale, offsetX, offsetY);
}

ClientEntityPtr ClientCaveTile::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	return ClientEntityPtr(
			new ClientCaveTile(ctx->id, ctx->sprite, ctx->animation, ctx->x, ctx->y, ctx->width, ctx->height, ctx->soundMapping, ctx->align));
}

ClientCaveTile::Factory ClientCaveTile::FACTORY;
