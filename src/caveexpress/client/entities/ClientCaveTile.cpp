#include "ClientCaveTile.h"
#include "sound/Sound.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"

namespace caveexpress {

ClientCaveTile::ClientCaveTile (uint16_t id, const std::string& sprite, const Animation& animation, float x,
		float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align) :
		ClientMapTile(EntityTypes::CAVE, id, sprite, animation, x, y, sizeX, sizeY, 0, soundMapping, align), _lightState(
				DEFAULT_LIGHT_STATE), _sprite(sprite)
{
	const SoundType& soundType = SoundTypes::SOUND_AMBIENT_CAVE;
	_animationSound = SoundControl.play(soundType.getSound(), getPos(), soundType.isLoopSound());
}

ClientCaveTile::~ClientCaveTile ()
{
}

void ClientCaveTile::render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY) const
{
	if (_currSprite) {
		if (_lightState)
			_currSprite->setCurrentFrame(0);
		else
			_currSprite->setCurrentFrame(1);
	}

	ClientMapTile::render(frontend, layer, scale, zoom, offsetX, offsetY);
}

ClientEntityPtr ClientCaveTile::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	return ClientEntityPtr(
			new ClientCaveTile(ctx->id, ctx->sprite, ctx->animation, ctx->x, ctx->y, ctx->width, ctx->height, ctx->soundMapping, ctx->align));
}

ClientCaveTile::Factory ClientCaveTile::FACTORY;

}
