#include "CavePacker.h"
#include "engine/client/ui/UI.h"
#include "cavepacker/client/ui/windows/UIMainWindow.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"
#include "cavepacker/client/ui/windows/intro/IntroGame.h"
#include "cavepacker/client/CavePackerClientMap.h"
#include "cavepacker/server/network/SpawnHandler.h"
#include "cavepacker/server/network/DisconnectHandler.h"
#include "cavepacker/server/network/StartMapHandler.h"
#include "cavepacker/server/network/MovementHandler.h"
#include "cavepacker/server/network/StopMovementHandler.h"
#include "cavepacker/server/network/ClientInitHandler.h"
#include "cavepacker/server/network/ErrorHandler.h"
#include "cavepacker/server/network/StopFingerMovementHandler.h"
#include "cavepacker/server/network/FingerMovementHandler.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "engine/client/entities/ClientEntityFactory.h"
#include "engine/client/entities/ClientMapTile.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/common/campaign/ICampaignManager.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/System.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/Commands.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/network/messages/LoadMapMessage.h"
#include "engine/common/network/messages/FinishedMapMessage.h"
#include "engine/client/ui/windows/UICampaignMapWindow.h"
#include "cavepacker/client/ui/windows/UICavePackerMapOptionsWindow.h"
#include "engine/client/ui/windows/UIPaymentWindow.h"
#include "engine/client/ui/windows/UISettingsWindow.h"
#include "engine/client/ui/windows/UIMapFinishedWindow.h"
#include "engine/common/campaign/persister/SQLitePersister.h"

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
	return new FileMapManager("sok");
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

	const bool isDone = _map.isDone();
	if (isDone && !_map.isRestartInitialized()) {
		const uint32_t finishPoints = _map.isAutoSolve() ? 0 : _map.getMoves();
		const uint32_t pushes = _map.isAutoSolve() ? 0 : _map.getPushes();
		const uint8_t stars = getStars();
		_campaignManager->getAutoActiveCampaign();
		if (!_campaignManager->updateMapValues(_map.getName(), finishPoints, pushes, stars, true))
			error(LOG_SERVER, "Could not save the values for the map");

		System.track("MapState", String::format("finished: %s with %i moves and %i pushes - got %i stars", _map.getName().c_str(), finishPoints, pushes, stars));
		const FinishedMapMessage msg(_map.getName(), finishPoints, pushes, stars);
		_serviceProvider->getNetwork().sendToAllClients(msg);
	} else if (!isDone && _map.isFailed()) {
		debug(LOG_SERVER, "map failed");
		const uint32_t delay = 1000;
		_map.restart(delay);
	}
}

uint8_t CavePacker::getStars () const {
	if (_map.isAutoSolve())
		return 0;
	const int bestMoves = _map.getBestMoves();
	if (bestMoves == 0)
		return 3;

	const uint32_t finishPoints = _map.getMoves();
	const float p = finishPoints * 100.0f / static_cast<float>(bestMoves);
	info(LOG_SERVER, "pushes to best pushes => " + string::toString(p));
	if (p < 120.0f)
		return 3;
	if (p < 130.0f)
		return 2;
	if (p < 160.0f)
		return 1;
	return 0;
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
		const ConfigVarPtr& persister = Config.get().getConfigVar("persister", "sqlite", true, CV_READONLY);
		if (persister->getValue() == "nop")
			_persister = new NOPPersister();
		else
			_persister = new SQLitePersister(System.getDatabaseDirectory() + "gamestate.sqlite");
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
	r.registerFactory(EntityTypes::PLAYER, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::TARGET, ClientMapTile::FACTORY);

	ProtocolHandlerRegistry& rp = ProtocolHandlerRegistry::get();
	rp.registerServerHandler(protocol::PROTO_SPAWN, new SpawnHandler(_map));
	rp.registerServerHandler(protocol::PROTO_DISCONNECT, new DisconnectHandler(_map));
	rp.registerServerHandler(protocol::PROTO_STARTMAP, new StartMapHandler(_map));
	rp.registerServerHandler(protocol::PROTO_MOVEMENT, new MovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_FINGERMOVEMENT, new FingerMovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_STOPFINGERMOVEMENT, new StopFingerMovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_STOPMOVEMENT, new StopMovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_ERROR, new ErrorHandler(_map));
	rp.registerServerHandler(protocol::PROTO_CLIENTINIT, new ClientInitHandler(_map));

	_campaignManager->getAutoActiveCampaign();
}

void CavePacker::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	UI& ui = UI::get();
	ui.addWindow(new UIMainWindow(frontend));
	CavePackerClientMap *map = new CavePackerClientMap(0, 0, frontend->getWidth(), frontend->getHeight(), frontend, serviceProvider, UI::get().loadTexture("tile-reference")->getWidth());
	_clientMap = map;
	ui.addWindow(new UIMapWindow(frontend, serviceProvider, *_campaignManager, *_clientMap));
	ui.addWindow(new UICampaignMapWindow(frontend, *_campaignManager));
	ui.addWindow(new UIPaymentWindow(frontend));
	UISettingsWindow* settings = new UISettingsWindow(frontend, serviceProvider);
	settings->init();
	ui.addWindow(settings);
	ui.addWindow(new UIMapFinishedWindow(frontend, *_campaignManager, serviceProvider, SoundType::NONE));
	ui.addWindow(new IntroGame(frontend));
	ui.addWindow(new UICavePackerMapOptionsWindow(frontend, serviceProvider));
}

bool CavePacker::visitEntity (IEntity *entity)
{
	if (entity->isDynamic()) {
		_map.updateEntity(0, *entity);
	}
	return false;
}
