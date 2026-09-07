#include "cavepacker/client/ClientMapHandlers.h"
#include "cavepacker/client/CavePackerClientMap.h"
#include "client/network/AddEntityHandler.h"
#include "client/network/InitDoneHandler.h"
#include "network/ProtocolHandlerRegistry.h"

namespace cavepacker {

void registerClientMapHandlers (CavePackerClientMap& map)
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.unregisterClientHandler(::protocol::PROTO_ADDENTITY);
	r.registerClientHandler(::protocol::PROTO_ADDENTITY, new AddEntityHandler(map));
	r.unregisterClientHandler(::protocol::PROTO_INITDONE);
	r.registerClientHandler(::protocol::PROTO_INITDONE, new InitDoneHandler(map));
}

}
