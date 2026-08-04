#include "ClientGate.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "sprites/Sprite.h"
#include "common/Math.h"
#include "common/Layer.h"

namespace caveexpress {

ClientGate::ClientGate (uint16_t id, const std::string& sprite, const Animation& animation, float x,
		float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align) :
		ClientMapTile(EntityTypes::GATE, id, sprite, animation, x, y, sizeX, sizeY, 0, soundMapping, align),
		_openAmount(255)
{
	applyOpenFrame();
}

ClientGate::~ClientGate ()
{
}

void ClientGate::applyOpenFrame () const
{
	if (!_currSprite)
		return;
	int frames = _currSprite->getFrameCount();
	if (frames <= 1) {
		for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
			frames = std::max(frames, _currSprite->getFrameCount(layer));
		}
	}
	if (frames <= 1)
		return;
	// 255 = fully closed (last frame), 0 = fully open (first frame)
	const int frame = static_cast<int>(_openAmount * (frames - 1) / 255.0f + 0.5f);
	_currSprite->setCurrentFrame(clamp(frame, 0, frames - 1));
}

void ClientGate::setOpenAmount (uint8_t openAmount)
{
	_openAmount = openAmount;
	applyOpenFrame();
}

bool ClientGate::update (uint32_t deltaTime, bool lerpPos, bool animateSpriteAlways)
{
	const bool result = ClientMapTile::update(deltaTime, lerpPos, animateSpriteAlways);
	// Parent idle logic forces frame 0 (open); keep protrusion-driven frame.
	applyOpenFrame();
	return result;
}

void ClientGate::render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY, int mapPixelWidth, int mapPixelHeight) const
{
	applyOpenFrame();
	ClientMapTile::render(frontend, layer, scale, zoom, offsetX, offsetY, mapPixelWidth, mapPixelHeight);
}

ClientEntityPtr ClientGate::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	return ClientEntityPtr(
			new ClientGate(ctx->id, ctx->sprite, ctx->animation, ctx->x, ctx->y, ctx->width, ctx->height, ctx->soundMapping, ctx->align));
}

ClientGate::Factory ClientGate::FACTORY;

}
