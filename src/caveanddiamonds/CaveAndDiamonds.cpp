#include "CaveAndDiamonds.h"
#include "engine/client/ui/UI.h"
#include "caveanddiamonds/client/ui/windows/UIMainWindow.h"
#include "caveanddiamonds/client/ui/windows/UIMapWindow.h"
#include "caveanddiamonds/client/ui/windows/intro/IntroGame.h"
#include "caveanddiamonds/client/CaveAndDiamondsClientMap.h"
#include "caveanddiamonds/server/network/SpawnHandler.h"
#include "caveanddiamonds/server/network/DisconnectHandler.h"
#include "caveanddiamonds/server/network/StartMapHandler.h"
#include "caveanddiamonds/server/network/MovementHandler.h"
#include "caveanddiamonds/server/network/StopMovementHandler.h"
#include "caveanddiamonds/server/network/ClientInitHandler.h"
#include "caveanddiamonds/server/network/ErrorHandler.h"
#include "caveanddiamonds/server/network/StopFingerMovementHandler.h"
#include "caveanddiamonds/server/network/FingerMovementHandler.h"
#include "caveanddiamonds/shared/CaveAndDiamondsEntityType.h"
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
#include "caveanddiamonds/client/ui/windows/UICaveAndDiamondsMapOptionsWindow.h"
#include "engine/client/ui/windows/UIPaymentWindow.h"
#include "engine/client/ui/windows/UISettingsWindow.h"
#include "engine/client/ui/windows/UIMapFinishedWindow.h"
#include "engine/client/ui/windows/UIGestureWindow.h"
#include "engine/common/campaign/persister/SQLitePersister.h"
#include <SDL.h>

CaveAndDiamonds::CaveAndDiamonds ():
	_persister(nullptr), _campaignManager(nullptr), _clientMap(nullptr), _frontend(nullptr), _serviceProvider(nullptr)
{
	delete _persister;
	delete _campaignManager;
	delete _clientMap;
}

CaveAndDiamonds::~CaveAndDiamonds ()
{
}

IMapManager* CaveAndDiamonds::getMapManager ()
{
	return new FileMapManager("sok");
}

void CaveAndDiamonds::update (uint32_t deltaTime)
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
		const uint32_t moves = _map.getMoves();
		const uint32_t pushes = _map.getPushes();
		const uint8_t stars = getStars();
		_campaignManager->getAutoActiveCampaign();
		if (!_campaignManager->updateMapValues(_map.getName(), moves, pushes, stars, true))
			error(LOG_SERVER, "Could not save the values for the map");

		if (_map.getPlayers().size() == 1) {
			const Player* player = _map.getPlayers()[0];
			const std::string& solution = player->getSolution();
			info(LOG_SERVER, "solution: " + solution);
#if SDL_VERSION_ATLEAST(2, 0, 0)
			SDL_SetClipboardText(solution.c_str());
#endif
			if (!_campaignManager->addAdditionMapData(_map.getName(), solution))
				error(LOG_SERVER, "Could not save the solution for the map");
		} else {
			info(LOG_SERVER, "no solution in multiplayer games");
		}

		System.track("MapState", String::format("finished: %s with %i moves and %i pushes - got %i stars", _map.getName().c_str(), moves, pushes, stars));
		_map.abortAutoSolve();
		const FinishedMapMessage msg(_map.getName(), moves, pushes, stars);
		_serviceProvider->getNetwork().sendToAllClients(msg);
	} else if (!isDone && _map.isFailed()) {
		debug(LOG_SERVER, "map failed");
		const uint32_t delay = 1000;
		_map.restart(delay);
	}
}

uint8_t CaveAndDiamonds::getStars () const {
	if (_map.isAutoSolve())
		return 0;
	const int bestMoves = _map.getBestMoves();
	if (bestMoves == 0)
		return 3;

	const uint32_t finishPoints = _map.getMoves();
	if (finishPoints == 0)
		return 0;
	const float p = finishPoints * 100.0f / static_cast<float>(bestMoves);
	info(LOG_SERVER, String::format("best pushes: %i, your pushes: %i => pushes to best pushes: %f", bestMoves, finishPoints, p));
	if (p < 120.0f)
		return 3;
	if (p < 130.0f)
		return 2;
	if (p < 160.0f)
		return 1;
	return 0;
}

bool CaveAndDiamonds::mapLoad (const std::string& map)
{
	return _map.load(map);
}

void CaveAndDiamonds::mapReload ()
{
	_map.reload();
}

void CaveAndDiamonds::mapShutdown ()
{
	_map.shutdown();
}

void CaveAndDiamonds::connect (ClientId clientId)
{
	const LoadMapMessage msg(_map.getName(), _map.getTitle());
	_serviceProvider->getNetwork().sendToClients(ClientIdToClientMask(clientId), msg);
}

int CaveAndDiamonds::disconnect (ClientId clientId)
{
	_map.removePlayer(clientId);
	return 0;
}

int CaveAndDiamonds::getPlayers ()
{
	return _map.getPlayers().size();
}

std::string CaveAndDiamonds::getMapName ()
{
	return _map.getName();
}

void CaveAndDiamonds::shutdown ()
{
	mapShutdown();
}

void CaveAndDiamonds::init (IFrontend *frontend, ServiceProvider& serviceProvider)
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
	r.registerFactory(EntityTypes::STONE, ClientEntity::FACTORY);
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

void CaveAndDiamonds::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	UI& ui = UI::get();
	ui.addWindow(new UIMainWindow(frontend));
	CaveAndDiamondsClientMap *map = new CaveAndDiamondsClientMap(0, 0, frontend->getWidth(), frontend->getHeight(), frontend, serviceProvider, UI::get().loadTexture("tile-reference")->getWidth());
	_clientMap = map;
	ui.addWindow(new UIMapWindow(frontend, serviceProvider, *_campaignManager, *_clientMap));
	ui.addWindow(new UICampaignMapWindow(frontend, *_campaignManager));
	ui.addWindow(new UIPaymentWindow(frontend));
	UISettingsWindow* settings = new UISettingsWindow(frontend, serviceProvider);
	settings->init();
	ui.addWindow(settings);
	ui.addWindow(new UIGestureWindow(frontend));
	ui.addWindow(new UIMapFinishedWindow(frontend, *_campaignManager, serviceProvider, SoundType::NONE));
	ui.addWindow(new IntroGame(frontend));
	ui.addWindow(new UICaveAndDiamondsMapOptionsWindow(frontend, serviceProvider));
}

bool CaveAndDiamonds::visitEntity (IEntity *entity)
{
	if (entity->isDynamic()) {
		_map.updateEntity(0, *entity);
	}
	return false;
}
