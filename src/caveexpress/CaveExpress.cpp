#include "CaveExpress.h"
#include "engine/client/sound/Sound.h"
#include "engine/common/Singleton.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Commands.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/ExecutionTime.h"
#include "engine/client/entities/ClientEntityFactory.h"
#include "engine/client/entities/ClientPlayer.h"
#include "engine/client/entities/ClientMapTile.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/client/ui/nodes/IUINodeMap.h"
#include "engine/client/ui/windows/IUIMapWindow.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/windows/UIPaymentWindow.h"
#include "engine/client/ui/windows/UIMultiplayerWindow.h"
#include "engine/client/ui/windows/UICampaignWindow.h"
#include "engine/client/ui/windows/UICampaignMapWindow.h"
#include "engine/client/ui/windows/UICreateServerWindow.h"
#include "engine/common/campaign/ICampaignManager.h"
#include "engine/common/Shared.h"
#include "engine/common/Logger.h"
#include "engine/common/SQLite.h"
#include "engine/common/network/messages/LoadMapMessage.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "caveexpress/client/ui/windows/UIMapFailedWindow.h"
#include "caveexpress/client/entities/ClientWindowTile.h"
#include "caveexpress/client/entities/ClientCaveTile.h"
#include "caveexpress/client/entities/ClientParticle.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/client/commands/CmdDrop.h"
#include "caveexpress/shared/constants/Commands.h"
#include "caveexpress/client/ui/windows/UIMainWindow.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "engine/client/ui/windows/UIMapOptionsWindow.h"
#include "engine/client/ui/windows/UISettingsWindow.h"
#include "engine/client/ui/windows/UIGestureWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorWindow.h"
#include "caveexpress/client/ui/windows/UIGameHelpWindow.h"
#include "caveexpress/client/ui/windows/UIGameOverWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorHelpWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorOptionsWindow.h"
#include "caveexpress/client/ui/windows/UIGameFinishedWindow.h"
#include "engine/client/ui/windows/UIMapFinishedWindow.h"
#include "caveexpress/client/ui/windows/UICaveExpressSettingsWindow.h"
#include "engine/client/ui/windows/UIModeSelectionWindow.h"
#include "caveexpress/client/ui/windows/intro/IntroPackage.h"
#include "caveexpress/client/ui/windows/intro/IntroTime.h"
#include "caveexpress/client/ui/windows/intro/IntroTree.h"
#include "caveexpress/client/ui/windows/intro/IntroGeyser.h"
#include "caveexpress/client/ui/windows/intro/IntroAttack.h"
#include "caveexpress/client/ui/windows/intro/IntroFlying.h"
#include "caveexpress/client/ui/windows/intro/IntroFindYourWay.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/client/CaveExpressClientMap.h"
#include "caveexpress/client/commands/CmdMapOpenInEditor.h"
#include "caveexpress/client/network/AddRopeHandler.h"
#include "caveexpress/client/network/RemoveRopeHandler.h"
#include "caveexpress/client/network/AddEntityWithSoundHandler.h"
#include "caveexpress/client/network/WaterHeightHandler.h"
#include "caveexpress/client/network/UpdateCollectedTypeHandler.h"
#include "caveexpress/client/network/SpawnInfoHandler.h"
#include "caveexpress/client/network/WaterImpactHandler.h"
#include "caveexpress/client/network/AddCaveHandler.h"
#include "caveexpress/client/network/LightStateHandler.h"
#include "caveexpress/client/network/HudInitDoneHandler.h"
#include "caveexpress/client/network/UpdateParticleHandler.h"
#include "caveexpress/client/network/UpdatePackageCountHandler.h"
#include "caveexpress/client/network/FailedMapHandler.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/server/entities/Fruit.h"
#include "caveexpress/shared/constants/Commands.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/server/network/SpawnHandler.h"
#include "caveexpress/server/network/DisconnectHandler.h"
#include "caveexpress/server/network/DropHandler.h"
#include "caveexpress/server/network/StartMapHandler.h"
#include "caveexpress/server/network/FingerMovementHandler.h"
#include "caveexpress/server/network/StopFingerMovementHandler.h"
#include "caveexpress/server/network/MovementHandler.h"
#include "caveexpress/server/network/StopMovementHandler.h"
#include "caveexpress/server/network/ClientInitHandler.h"
#include "caveexpress/server/network/ErrorHandler.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "engine/common/System.h"
#include "engine/common/network/INetwork.h"
#include "caveexpress/shared/CaveExpressSoundType.h"

CaveExpress::CaveExpress () :
		_persister(nullptr), _campaignManager(nullptr), _clientMap(nullptr), _updateEntitiesTime(0), _frontend(nullptr), _serviceProvider(nullptr),_connectedClients(
				0), _packageCount(0), _loadDelay(0), _loadDelayName("")
{
}

CaveExpress::~CaveExpress ()
{
	if (_persister != nullptr)
		delete _persister;
	if (_campaignManager != nullptr)
		delete _campaignManager;
	delete _clientMap;
}

IMapManager* CaveExpress::getMapManager ()
{
	return new LUAMapManager();
}

/**
 * @param[in] deltaTime The milliseconds since the last frame was executed
 * @return If @c true is returned, the next map in the campaign is going to be started.
 * @c false is returned if the map is still running or whenever it failed.
 */
void CaveExpress::update (uint32_t deltaTime)
{
	if (!_map.isActive() || deltaTime == 0)
		return;

	if (!_serviceProvider->getNetwork().isServer()) {
		_map.resetCurrentMap();
		return;
	}

	if (_map.isPause())
		return;

	_updateEntitiesTime -= deltaTime;
	_map.update(deltaTime);

	if (_updateEntitiesTime <= 0) {
		_packageCount = _map.countPackages();
		_map.visitEntities(this);
		_updateEntitiesTime = Constant::DELTA_PHYSICS_MILLIS;
	}

	if (_loadDelay > 0) {
		_loadDelay -= deltaTime;

		if (_loadDelay <= 0) {
			mapLoad(_loadDelayName);
			return;
		}

		return;
	}

	if (_map.handleDeadPlayers() > 0 && !_map.isActive()) {
		info(LOG_SERVER, "reset the game state");
		_campaignManager->reset();
		return;
	}

	const bool isDone = _map.isDone();
	if (isDone && !_map.isRestartInitialized()) {
		const uint32_t playTime = _map.getTime();
		const uint32_t referenceTime = _map.getReferenceTime();
		const float relativeRefTime = ConfigManager::get().getReferenceTimeFactor() * referenceTime;
		// this max is needed for debug reasons (if you force a map win)
		const float time = std::max(1.0f, playTime / 1000.0f);
		const uint32_t timeSeconds = std::max(1U, playTime / 1000U);
		const float pointsRate = relativeRefTime / time;
		const uint32_t timePoints = pointsRate * _map.getFinishPoints();
		const uint32_t finishPoints = timePoints + _map.getPoints();
		const int percent = time * 100.0f / relativeRefTime;
		info(LOG_SERVER, String::format(
						"seconds: %.0f, refseconds: %i, rate: %f, refpoints: %i, timePoints: %i, finishPoints: %i, percent: %i",
						time, _map.getReferenceTime(), pointsRate, _map.getFinishPoints(), timePoints, finishPoints, percent));
		_map.sendSound(0, SoundTypes::SOUND_MUSIC_WIN);
		uint8_t stars = 0;
		if (percent <= 70) {
			stars = 3;
		} else if (percent <= 90) {
			stars = 2;
		} else if (percent <= 100) {
			stars = 1;
		}
		if (!_campaignManager->updateMapValues(_map.getName(), finishPoints, timeSeconds, stars))
			error(LOG_SERVER, "Could not save the values for the map");

		System.track("MapState", String::format("finished: %s with %i points in %i seconds and with %i stars", _map.getName().c_str(), finishPoints, timeSeconds, stars));
		GameEvent.finishedMap(_map.getName(), finishPoints, timeSeconds, stars);
	} else if (!isDone && _map.isFailed()) {
		debug(LOG_SERVER, "map failed");
		const uint32_t delay = 1000;
		_map.restart(delay);
	}
}

bool CaveExpress::mapLoad (const std::string& map)
{
	_loadDelay = 0;
	_updateEntitiesTime = 0;
	_packageCount = 0;
	return _map.load(map);
}

void CaveExpress::mapReload ()
{
	_map.reload();
}

void CaveExpress::mapShutdown ()
{
	_map.shutdown();
}

void CaveExpress::connect (ClientId clientId)
{
	_connectedClients++;
	const LoadMapMessage msg(_map.getName(), _map.getTitle());
	_serviceProvider->getNetwork().sendToClients(ClientIdToClientMask(clientId), msg);
}

int CaveExpress::disconnect (ClientId clientId)
{
	_map.removePlayer(clientId);
	_connectedClients--;
	if (_connectedClients < 0) {
		_connectedClients = 0;
		error(LOG_SERVER, "client counts are out of sync");
	}

	return _connectedClients;
}

int CaveExpress::getPlayers ()
{
	return _map.getPlayers().size();
}

std::string CaveExpress::getMapName ()
{
	return _map.getName();
}

void CaveExpress::shutdown ()
{
	_map.shutdown();
}

void CaveExpress::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	ClientEntityRegistry &r = Singleton<ClientEntityRegistry>::getInstance();
	r.registerFactory(EntityTypes::DECORATION, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::SOLID, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::LAVA, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::GROUND, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::CAVE, ClientCaveTile::FACTORY);
	r.registerFactory(EntityTypes::WINDOW, ClientWindowTile::FACTORY);
	r.registerFactory(EntityTypes::NPC_FRIENDLY_GRANDPA, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::NPC_FRIENDLY_WOMAN, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::NPC_FRIENDLY_MAN, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::NPC_FISH, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::NPC_FLYING, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::NPC_WALKING, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::NPC_MAMMUT, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::NPC_BLOWING, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::PLAYER, ClientPlayer::FACTORY);
	r.registerFactory(EntityTypes::STONE, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::TREE, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::PACKAGE_ICE, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::PACKAGE_ROCK, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::PACKAGETARGET_ICE, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::PACKAGETARGET_ROCK, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::APPLE, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::BANANA, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::EGG, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::PARTICLE, ClientParticle::FACTORY);
	r.registerFactory(EntityTypes::GEYSER_ICE, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::GEYSER_ROCK, ClientEntity::FACTORY);
	r.registerFactory(EntityTypes::BOMB, ClientEntity::FACTORY);

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
	ProtocolHandlerRegistry& rp = ProtocolHandlerRegistry::get();
	rp.registerServerHandler(protocol::PROTO_SPAWN, new SpawnHandler(_map, _campaignManager));
	rp.registerServerHandler(protocol::PROTO_DISCONNECT, new DisconnectHandler(_map));
	rp.registerServerHandler(protocol::PROTO_STARTMAP, new StartMapHandler(_map));
	rp.registerServerHandler(protocol::PROTO_MOVEMENT, new MovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_FINGERMOVEMENT, new FingerMovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_STOPFINGERMOVEMENT, new StopFingerMovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_STOPMOVEMENT, new StopMovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_DROP, new DropHandler(_map));
	rp.registerServerHandler(protocol::PROTO_ERROR, new ErrorHandler(_map));
	rp.registerServerHandler(protocol::PROTO_CLIENTINIT, new ClientInitHandler(_map));
	_frontend = frontend;
	_serviceProvider = &serviceProvider;
	_map.init(_frontend, *_serviceProvider);
	GameEvent.init(*_serviceProvider);
}

void CaveExpress::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	assert(_campaignManager != nullptr);
	UI& ui = UI::get();
	CampaignManager& campaignMgr = *_campaignManager;
	ui.addWindow(new UIMainWindow(frontend, serviceProvider));
	ui.addWindow(new UICampaignWindow(frontend, serviceProvider, campaignMgr));
	ui.addWindow(new UICampaignMapWindow(frontend, campaignMgr));
	CaveExpressClientMap *map = new CaveExpressClientMap(0, 0, frontend->getWidth(), frontend->getHeight(), frontend, serviceProvider, UI::get().loadTexture("tile-reference")->getWidth());
	_clientMap = map;
	UIMapWindow *mapWindow = new UIMapWindow(frontend, serviceProvider, campaignMgr, *_clientMap);
	ui.addWindow(mapWindow);
	ui.addWindow(new UIModeSelectionWindow(frontend, campaignMgr));
	ui.addWindow(new UICaveExpressSettingsWindow(frontend, serviceProvider, campaignMgr));
	ui.addWindow(new IntroPackage(frontend));
	ui.addWindow(new IntroTime(frontend));
	ui.addWindow(new IntroTree(frontend));
	ui.addWindow(new IntroGeyser(frontend));
	ui.addWindow(new IntroAttack(frontend));
	ui.addWindow(new IntroFlying(frontend));
	ui.addWindow(new IntroFindYourWay(frontend));
	ui.addWindow(new UIPaymentWindow(frontend));
	ui.addWindow(new UIMultiplayerWindow(frontend, serviceProvider.getMapManager(), serviceProvider));
	ui.addWindow(new UICreateServerWindow(frontend, serviceProvider.getMapManager()));
	UIMapEditorWindow* mapEditorWindow = new UIMapEditorWindow(frontend, serviceProvider.getMapManager());
	ui.addWindow(mapEditorWindow);
	ui.addWindow(new UIGameHelpWindow(frontend));
	ui.addWindow(new UIGestureWindow(frontend));
	ui.addWindow(new UIGameOverWindow(frontend, campaignMgr));
	ui.addWindow(new UIMapOptionsWindow(frontend, serviceProvider));
	ui.addWindow(new UIGameFinishedWindow(frontend));
	ui.addWindow(new UIMapFinishedWindow(frontend, campaignMgr, serviceProvider, SoundTypes::SOUND_PACKAGE_COLLIDE));
	ui.addWindow(new UIMapFailedWindow(frontend, campaignMgr));
	ui.addWindow(new UIMapEditorHelpWindow(frontend));
	ui.addWindow(new UIMapEditorOptionsWindow(frontend, mapEditorWindow->getMapEditorNode()));

	Commands.registerCommand(CMD_DROP, new CmdDrop(*map));
	Commands.registerCommand(CMD_MAP_OPEN_IN_EDITOR, new CmdMapOpenInEditor(*map));

	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.unregisterClientHandler(protocol::PROTO_ADDROPE);
	r.registerClientHandler(protocol::PROTO_ADDROPE, new AddRopeHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_REMOVEROPE);
	r.registerClientHandler(protocol::PROTO_REMOVEROPE, new RemoveRopeHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_WATERHEIGHT);
	r.registerClientHandler(protocol::PROTO_WATERHEIGHT, new WaterHeightHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_UPDATEPACKAGECOUNT);
	r.registerClientHandler(protocol::PROTO_UPDATEPACKAGECOUNT, new UpdatePackageCountHandler());
	r.unregisterClientHandler(protocol::PROTO_UPDATECOLLECTEDTYPE);
	r.registerClientHandler(protocol::PROTO_UPDATECOLLECTEDTYPE, new UpdateCollectedTypeHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_WATERIMPACT);
	r.registerClientHandler(protocol::PROTO_WATERIMPACT, new WaterImpactHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_ADDCAVE);
	r.registerClientHandler(protocol::PROTO_ADDCAVE, new AddCaveHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_LIGHTSTATE);
	r.registerClientHandler(protocol::PROTO_LIGHTSTATE, new LightStateHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_ADDENTITY);
	r.registerClientHandler(protocol::PROTO_ADDENTITY, new AddEntityWithSoundHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_INITDONE);
	r.registerClientHandler(protocol::PROTO_INITDONE, new HudInitDoneHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_FAILEDMAP);
	r.registerClientHandler(protocol::PROTO_FAILEDMAP, new FailedMapHandler(*map, serviceProvider));
}

Map& CaveExpress::getMap ()
{
	return _map;
}

bool CaveExpress::visitEntity (IEntity *entity)
{
	if (!entity->isDynamic() && !entity->isWater() && !entity->isCave())
		return false;

	if (entity->isDirty()) {
		entity->snapshot();
		GameEvent.updateEntity(entity->getVisMask(), *entity);
	}
	return false;
}
