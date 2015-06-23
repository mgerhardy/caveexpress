#include "client/entities/ClientPlayer.h"
#include "common/Shared.h"
#include "common/Log.h"

ClientPlayer::ClientPlayer (const EntityType& type, uint16_t id, const Animation& animation, float x, float y,
		float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align) :
		ClientEntity(type, id, x, y, sizeX, sizeY, soundMapping, align, 0), _hasCollected(&EntityType::NONE)
{
	setAnimationType(animation);
}

ClientPlayer::~ClientPlayer ()
{
}

ClientEntityPtr ClientPlayer::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	return ClientEntityPtr(
			new ClientPlayer(ctx->type, ctx->id, ctx->animation, ctx->x, ctx->y, ctx->width, ctx->height, ctx->soundMapping, ctx->align));
}

ClientPlayer::Factory ClientPlayer::FACTORY;
