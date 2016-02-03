#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/RumbleMessage.h"
#include "client/ClientMap.h"

/**
 * @brief Informs the client that it could play a rumble effect on the map because the player hit something
 */
class RumbleHandler: public ClientProtocolHandler<RumbleMessage> {
private:
	ClientMap& _map;
public:
	RumbleHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const RumbleMessage* msg) override
	{
		_map.rumble(msg->getStrength(), msg->getLengthMillis());
	}
};
