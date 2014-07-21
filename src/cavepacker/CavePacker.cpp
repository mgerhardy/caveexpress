#include "CavePacker.h"
#include "engine/client/ui/UI.h"
#include "cavepacker/client/ui/windows/UIMainWindow.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"
#include "cavepacker/client/CavePackerClientMap.h"
#include "engine/client/entities/ClientPlayer.h"
#include "engine/client/entities/ClientMapTile.h"
#include "cavepacker/server/network/SpawnHandler.h"
#include "cavepacker/server/network/DisconnectHandler.h"
#include "cavepacker/server/network/StartMapHandler.h"
#include "cavepacker/server/network/MovementHandler.h"
#include "cavepacker/server/network/StopMovementHandler.h"
#include "cavepacker/server/network/ClientInitHandler.h"
#include "cavepacker/server/network/ErrorHandler.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "engine/client/entities/ClientEntityFactory.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/Commands.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/network/messages/LoadMapMessage.h"
#include "engine/client/network/InitDoneHandler.h"
#include "engine/client/network/AddEntityHandler.h"
#include "engine/client/ui/windows/UICampaignMapWindow.h"
#include "engine/client/ui/windows/UIMapOptionsWindow.h"

CavePacker::CavePacker ():
	_persister(nullptr), _campaignManager(nullptr), _clientMap(nullptr), _frontend(nullptr), _serviceProvider(nullptr)
{
	delete _persister;
	delete _campaignManager;
	delete _clientMap;
}

CavePacker::~CavePacker ()
{
}

IMapManager* CavePacker::getMapManager ()
{
	return new FileMapManager("map");
}

void CavePacker::update (uint32_t deltaTime)
{
	if (!_map.isActive() || deltaTime == 0)
		return;

	if (!_serviceProvider->getNetwork().isServer()) {
		_map.resetCurrentMap();
		return;
	}

	if (_map.isPause())
		return;

	_map.update(deltaTime);
}

bool CavePacker::mapLoad (const std::string& map)
{
	return _map.load(map);
}

void CavePacker::mapReload ()
{
	_map.reload();
}

void CavePacker::mapShutdown ()
{
	_map.shutdown();
}

void CavePacker::connect (ClientId clientId)
{
	const LoadMapMessage msg(_map.getName(), _map.getTitle());
	_serviceProvider->getNetwork().sendToClients(ClientIdToClientMask(clientId), msg);
}

int CavePacker::disconnect (ClientId clientId)
{
	_map.removePlayer(clientId);
	return 0;
}

int CavePacker::getPlayers ()
{
	return _map.getPlayers().size();
}

std::string CavePacker::getMapName ()
{
	return _map.getName();
}

void CavePacker::shutdown ()
{
	mapShutdown();
}

void CavePacker::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;

	{
		ExecutionTime e("loading persister");
		_persister = new NOPPersister();
	}
	{
		ExecutionTime e("campaign manager");
		_campaignManager = new CampaignManager(_persister, serviceProvider.getMapManager());
		_campaignManager->init();
	}

	_map.init(_frontend, *_serviceProvider);

	ClientEntityRegistry &r = Singleton<ClientEntityRegistry>::getInstance();
	r.registerFactory(EntityTypes::SOLID, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::GROUND, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::PACKAGE, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::PLAYER, ClientPlayer::FACTORY);
	r.registerFactory(EntityTypes::TARGET, ClientMapTile::FACTORY);

	ProtocolHandlerRegistry& rp = ProtocolHandlerRegistry::get();
	rp.registerServerHandler(protocol::PROTO_SPAWN, new SpawnHandler(_map));
	rp.registerServerHandler(protocol::PROTO_DISCONNECT, new DisconnectHandler(_map));
	rp.registerServerHandler(protocol::PROTO_STARTMAP, new StartMapHandler(_map));
	rp.registerServerHandler(protocol::PROTO_MOVEMENT, new MovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_STOPMOVEMENT, new StopMovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_ERROR, new ErrorHandler(_map));
	rp.registerServerHandler(protocol::PROTO_CLIENTINIT, new ClientInitHandler(_map));
}

void CavePacker::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	UI& ui = UI::get();
	ui.addWindow(new UIMainWindow(frontend));
	CavePackerClientMap *map = new CavePackerClientMap(0, 0, frontend->getWidth(), frontend->getHeight(), frontend, serviceProvider, UI::get().loadTexture("tile-reference")->getWidth());
	_clientMap = map;
	ui.addWindow(new UIMapWindow(frontend, serviceProvider, *_campaignManager, *_clientMap));
	ui.addWindow(new UICampaignMapWindow(frontend, *_campaignManager));
	ui.addWindow(new UIMapOptionsWindow(frontend, serviceProvider));

	ProtocolHandlerRegistry& rp = ProtocolHandlerRegistry::get();
	rp.unregisterClientHandler(protocol::PROTO_INITDONE);
	rp.registerClientHandler(protocol::PROTO_INITDONE, new InitDoneHandler(*_clientMap));
	rp.unregisterClientHandler(protocol::PROTO_ADDENTITY);
	rp.registerClientHandler(protocol::PROTO_ADDENTITY, new AddEntityHandler(*_clientMap));
}

bool CavePacker::visitEntity (IEntity *entity)
{
	return false;
}
