#include "ClientPressurePlate.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "common/String.h"

namespace caveexpress {

ClientPressurePlate::ClientPressurePlate (uint16_t id, const std::string& sprite, const Animation& animation, float x,
		float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align) :
		ClientMapTile(EntityTypes::PRESSUREPLATE, id, sprite, animation, x, y, sizeX, sizeY, 0, soundMapping, align)
{
}

ClientPressurePlate::~ClientPressurePlate ()
{
}

void ClientPressurePlate::setAnimationType (const Animation& animation)
{
	ClientEntity::setAnimationType(animation);
	std::string name = _sprite;
	if (animation == Animations::ANIMATION_ACTIVE) {
		name = string::replaceAll(name, "-idle", "-active");
	} else {
		name = string::replaceAll(name, "-active", "-idle");
	}
	setNewSprite(name);
}

ClientEntityPtr ClientPressurePlate::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	return ClientEntityPtr(
			new ClientPressurePlate(ctx->id, ctx->sprite, ctx->animation, ctx->x, ctx->y, ctx->width, ctx->height, ctx->soundMapping, ctx->align));
}

ClientPressurePlate::Factory ClientPressurePlate::FACTORY;

}
