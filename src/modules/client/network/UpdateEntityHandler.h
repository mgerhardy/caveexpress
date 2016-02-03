#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdateEntityMessage.h"
#include "client/ClientMap.h"

class UpdateEntityHandler: public ClientProtocolHandler<UpdateEntityMessage> {
private:
	ClientMap& _map;
public:
	UpdateEntityHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const UpdateEntityMessage* msg) override
	{
		_map.updateEntity(msg->getEntityId(), msg->getX(), msg->getY(), msg->getAngle(), msg->getState());
	}
};
