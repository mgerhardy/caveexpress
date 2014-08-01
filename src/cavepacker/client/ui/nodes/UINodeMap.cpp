#include "UINodeMap.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/client/network/AddEntityHandler.h"
#include "engine/client/network/InitDoneHandler.h"
#include "cavepacker/client/network/FinishedMapHandler.h"
#include "cavepacker/client/network/ClientAutoSolveHandler.h"

UINodeMap::UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, ClientMap& map) :
		IUINodeMap(frontend, serviceProvider, campaignManager, x, y, width, height, map)
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerClientHandler(protocol::PROTO_ADDENTITY, new AddEntityHandler(_map));
	r.registerClientHandler(protocol::PROTO_INITDONE, new InitDoneHandler(_map));
	r.registerClientHandler(protocol::PROTO_FINISHEDMAP, new FinishedMapHandler(_map));
	r.registerClientHandler(protocol::PROTO_AUTOSOLVE, new ClientAutoSolveHandler());
}

UINodeMap::~UINodeMap ()
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.unregisterClientHandler(protocol::PROTO_ADDENTITY);
	r.unregisterClientHandler(protocol::PROTO_INITDONE);
	r.unregisterClientHandler(protocol::PROTO_FINISHEDMAP);
	r.unregisterClientHandler(protocol::PROTO_AUTOSOLVE);
}
