#include "UINodeMap.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "caveexpress/client/network/SpawnInfoHandler.h"
#include "caveexpress/client/network/UpdateParticleHandler.h"
#include "caveexpress/client/network/UpdatePackageCountHandler.h"

UINodeMap::UINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, ClientMap& map) :
		IUINodeMap(frontend, serviceProvider, campaignManager, x, y, width, height, map)
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerClientHandler(protocol::PROTO_SPAWNINFO, new SpawnInfoHandler(_map, this));
	r.registerClientHandler(protocol::PROTO_UPDATEPARTICLE, new UpdateParticleHandler(_map));
}

UINodeMap::~UINodeMap ()
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.unregisterClientHandler(protocol::PROTO_SPAWNINFO);
	r.unregisterClientHandler(protocol::PROTO_UPDATEPARTICLE);
}
