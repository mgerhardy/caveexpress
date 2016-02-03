#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/ChangeAnimationMessage.h"
#include "client/ClientMap.h"

class ChangeAnimationHandler: public ClientProtocolHandler<ChangeAnimationMessage> {
private:
	ClientMap& _map;
public:
	ChangeAnimationHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const ChangeAnimationMessage* msg) override
	{
		const uint16_t id = msg->getEntityId();
		const Animation& animation = msg->getAnimation();
		_map.changeAnimation(id, animation);
	}
};
