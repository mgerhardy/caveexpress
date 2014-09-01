#pragma once

#include "shared/network/IProtocolHandler.h"
#include "shared/network/messages/SoundMessage.h"
#include "client/sound/Sound.h"
#include "shared/Logger.h"

class SoundHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const SoundMessage *msg = static_cast<const SoundMessage*>(&message);
		const SoundType sound = msg->getSoundType();
		const std::string snd = sound::getSoundByType(sound);
		if (snd.empty()) {
			error(LOG_CLIENT, String::format("no sound found for type %i", static_cast<int>(sound)));
			return;
		}

		const float xpos = msg->getX();
		const float ypos = msg->getY();
		const vec2 position(xpos, ypos);
		const bool loop = sound::isLoopSound(sound);
		debug(LOG_CLIENT, "play sound " + snd + (loop ? " as loop sound" : " without looping"));
		SoundControl.play(snd, position, loop);
	}
};
