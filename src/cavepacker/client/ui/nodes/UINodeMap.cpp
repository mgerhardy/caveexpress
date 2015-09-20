#include "UINodeMap.h"
#include "network/ProtocolHandlerRegistry.h"
#include "client/network/AddEntityHandler.h"
#include "client/network/InitDoneHandler.h"
#include "cavepacker/client/network/ClientAutoSolveHandler.h"
#include "cavepacker/client/network/ClientShowDeadlocksHandler.h"
#include "cavepacker/client/commands/CmdUndo.h"

namespace cavepacker {

UINodeMap::UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, CavePackerClientMap& map) :
		IUINodeMap(frontend, serviceProvider, campaignManager, x, y, width, height, map)
{
	Commands.registerCommand("undo", new CmdUndo(map));

	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerClientHandler(::protocol::PROTO_ADDENTITY, new AddEntityHandler(_map));
	r.registerClientHandler(::protocol::PROTO_INITDONE, new InitDoneHandler(_map));
	r.registerClientHandler(protocol::PROTO_AUTOSOLVE, new ClientAutoSolveHandler(true));
	r.registerClientHandler(protocol::PROTO_AUTOSOLVEABORT, new ClientAutoSolveHandler(false));
	r.registerClientHandler(protocol::PROTO_SHOWDEADLOCKS, new ClientShowDeadlocksHandler(map));
}

UINodeMap::~UINodeMap ()
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.unregisterClientHandler(::protocol::PROTO_ADDENTITY);
	r.unregisterClientHandler(::protocol::PROTO_INITDONE);
	r.unregisterClientHandler(protocol::PROTO_AUTOSOLVE);
	r.unregisterClientHandler(protocol::PROTO_AUTOSOLVEABORT);
	r.unregisterClientHandler(protocol::PROTO_SHOWDEADLOCKS);
}

}
