#include "ClientWindowTile.h"
#include "caveexpress/shared/CaveExpressEntityType.h"

ClientWindowTile::ClientWindowTile (uint16_t id, const std::string& sprite, const Animation& animation, float x,
		float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align) :
		ClientMapTile(EntityTypes::WINDOW, id, sprite, animation, x, y, sizeX, sizeY, 0, soundMapping, align), _sprite(sprite), _lightState(
				DEFAULT_LIGHT_STATE)
{
}

ClientWindowTile::~ClientWindowTile ()
{
}

void ClientWindowTile::render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY) const
{
	if (_lightState)
		_currSprite->setCurrentFrame(0);
	else
		_currSprite->setCurrentFrame(1);

	ClientMapTile::render(frontend, layer, scale, zoom, offsetX, offsetY);
}

ClientEntityPtr ClientWindowTile::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	return ClientEntityPtr(
			new ClientWindowTile(ctx->id, ctx->sprite, ctx->animation, ctx->x, ctx->y, ctx->width, ctx->height, ctx->soundMapping, ctx->align));
}

ClientWindowTile::Factory ClientWindowTile::FACTORY;
