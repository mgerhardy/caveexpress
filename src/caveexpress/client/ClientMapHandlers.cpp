#include "caveexpress/client/ClientMapHandlers.h"
#include "caveexpress/client/CaveExpressClientMap.h"
#include "caveexpress/client/network/AddCaveHandler.h"
#include "caveexpress/client/network/AddEntityWithSoundHandler.h"
#include "caveexpress/client/network/GateStateHandler.h"
#include "caveexpress/client/network/LightStateHandler.h"
#include "caveexpress/client/network/WaterHeightHandler.h"
#include "client/network/InitDoneHandler.h"
#include "network/ProtocolHandlerRegistry.h"

namespace caveexpress {

void registerClientMapHandlers (CaveExpressClientMap& map)
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.unregisterClientHandler(::protocol::PROTO_ADDENTITY);
	r.registerClientHandler(::protocol::PROTO_ADDENTITY, new AddEntityWithSoundHandler(map));
	r.unregisterClientHandler(::protocol::PROTO_INITDONE);
	r.registerClientHandler(::protocol::PROTO_INITDONE, new InitDoneHandler(map));
	r.unregisterClientHandler(protocol::PROTO_WATERHEIGHT);
	r.registerClientHandler(protocol::PROTO_WATERHEIGHT, new WaterHeightHandler(map));
	r.unregisterClientHandler(protocol::PROTO_ADDCAVE);
	r.registerClientHandler(protocol::PROTO_ADDCAVE, new AddCaveHandler(map));
	r.unregisterClientHandler(protocol::PROTO_LIGHTSTATE);
	r.registerClientHandler(protocol::PROTO_LIGHTSTATE, new LightStateHandler(map));
	r.unregisterClientHandler(protocol::PROTO_GATESTATE);
	r.registerClientHandler(protocol::PROTO_GATESTATE, new GateStateHandler(map));
}

}
