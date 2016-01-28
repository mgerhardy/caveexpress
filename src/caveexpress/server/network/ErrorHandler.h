#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/ErrorMessage.h"
#include "caveexpress/server/map/Map.h"

namespace caveexpress {

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
		const ErrorMessage* msg = assert_cast<const ErrorMessage*, const IProtocolMessage*>(&message);
		switch (msg->getErrorType()) {
		case UNKNOWN_ENTITY: {
			const IEntity* entity = _map.getEntity(msg->getErrorData());
			if (entity)
				Log::error(LOG_GAMEIMPL, "client doesn't know about the entity %i of the type %s", msg->getErrorData(),
								entity->getType().name.c_str());
			else
				Log::error(LOG_GAMEIMPL, "client owns a reference to an entity that does not exist on the server side");
			break;
		}
		default:
			Log::error(LOG_GAMEIMPL, "unknown error type given");
			break;
		}
	}
};

}
