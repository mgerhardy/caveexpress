#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/ErrorMessage.h"
#include "caveanddiamonds/server/map/Map.h"

class ErrorHandler: public IServerProtocolHandler {
private:
	Map& _map;
public:
	ErrorHandler (Map& map) :
			_map(map)
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override
	{
		const ErrorMessage* msg = static_cast<const ErrorMessage*>(&message);
		switch (msg->getErrorType()) {
		case UNKNOWN_ENTITY: {
			const IEntity* entity = _map.getEntity(msg->getErrorData());
			if (entity)
				error(LOG_SERVER,
						String::format("client doesn't know about the entity %i of the type %s", msg->getErrorData(),
								entity->getType().name.c_str()));
			else
				error(LOG_SERVER, "client owns a reference to an entity that does not exist on the server side");
			break;
		}
		default:
			error(LOG_SERVER, "unknown error type given");
			break;
		}
	}
};
