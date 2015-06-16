#include "ClientNPC.h"
#include "common/Logger.h"
#include "ui/UI.h"
#include "caveexpress/shared/constants/NPCState.h"

namespace caveexpress {

ClientNPC::ClientNPC(const EntityType& type, uint16_t id, const Animation& animation, float x, float y, float sizeX, float sizeY,
		const SoundMapping& soundMapping, EntityAlignment align) :
		ClientEntity(type, id, x, y, sizeX, sizeY, soundMapping, align, 0), _speechBubbleDelay(0) {
	setAnimationType(animation);
}

ClientNPC::~ClientNPC() {
}

bool ClientNPC::update(uint32_t deltaTime, bool lerpPos) {
	const bool val = ClientEntity::update(deltaTime, lerpPos);
	if (_speechBubbleDelay < _time) {
		_speechBubbleDelay = 0;
		removeOverlay(_targetCaveSprite);
	}
	return val;
}

void ClientNPC::changeState(uint8_t state) {
	if (_state != state) {
		_speechBubbleDelay = 0;
		removeOverlay(_targetCaveSprite);
	}
	ClientEntity::changeState(state);
}

void ClientNPC::setTargetCave(uint8_t caveNumber, short delay) {
	_speechBubbleDelay = _time + delay;
	const std::string& caveSprite = String::format("cavenumber%i", caveNumber);
	_targetCaveSprite = UI::get().loadSprite(caveSprite);
	addOverlay(_targetCaveSprite);
}

ClientEntityPtr ClientNPC::Factory::create(const ClientEntityFactoryContext *ctx) const {
	return ClientEntityPtr(
			new ClientNPC(ctx->type, ctx->id, ctx->animation, ctx->x, ctx->y, ctx->width, ctx->height, ctx->soundMapping, ctx->align));
}

ClientNPC::Factory ClientNPC::FACTORY;

}
