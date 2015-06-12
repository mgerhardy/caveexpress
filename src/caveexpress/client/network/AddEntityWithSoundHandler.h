#pragma once

#include "client/network/AddEntityHandler.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "sound/Sound.h"

namespace caveexpress {

class AddEntityWithSoundHandler: public AddEntityHandler {
public:
	AddEntityWithSoundHandler (ClientMap& map) :
			AddEntityHandler(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		AddEntityHandler::execute(message);
		const AddEntityMessage *msg = static_cast<const AddEntityMessage*>(&message);
		const std::string& sprite = msg->getSprite();
		const SpriteDefPtr& def = SpriteDefinition::get().getSpriteDefinition(sprite);
		if (def && SpriteTypes::isWaterFall(def->type) && !ThemeTypes::isIce(def->theme)) {
			const uint16_t id = msg->getEntityId();
			const SoundType& soundType = SoundTypes::SOUND_AMBIENT_WATERFALL;
			ClientEntityPtr entity = _map.getEntity(id);
			const int animationSound = SoundControl.play(soundType.getSound(), entity->getPos(), soundType.isLoopSound());
			entity->setAnimationSound(animationSound);
		}
	}
};

}
