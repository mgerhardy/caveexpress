#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/SoundMessage.h"
#include "client/ClientMap.h"
#include "sound/Sound.h"

class SoundHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const SoundMessage *msg = static_cast<const SoundMessage*>(&message);
		const SoundType& sound = msg->getSoundType();
		const std::string& snd = sound.getSound();
		if (snd.empty()) {
			Log::error2(LOG_CLIENT, "no sound found for type %i", static_cast<int>(sound));
			return;
		}

		const float xpos = msg->getX();
		const float ypos = msg->getY();
		const vec2 position(xpos, ypos);
		const bool loop = sound.isLoopSound();
		Log::debug2(LOG_CLIENT, "play sound %s %s", snd.c_str(), (loop ? " as loop sound" : " without looping"));
		SoundControl.play(snd, position, loop);
	}
};
