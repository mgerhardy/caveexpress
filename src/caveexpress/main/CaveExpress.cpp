#include "CaveExpress.h"

#include "common/Singleton.h"
#include "common/ConfigManager.h"
#include "common/Commands.h"
#include "common/CommandSystem.h"
#include "common/ExecutionTime.h"
#include "common/Shared.h"
#include "common/Log.h"
#include "common/SQLite.h"
#include "common/System.h"
#include "caveexpress/shared/constants/ConfigVars.h"

#include "sound/Sound.h"

#include "service/ServiceProvider.h"

#include "client/entities/ClientEntityFactory.h"
#include "client/entities/ClientPlayer.h"
#include "client/entities/ClientMapTile.h"

#include "network/INetwork.h"
#include "network/IProtocolMessage.h"
#include "network/ProtocolHandlerRegistry.h"
#include "network/messages/LoadMapMessage.h"
#include "network/ProtocolHandlerRegistry.h"

#include "ui/nodes/IUINodeMap.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/windows/IUIMapWindow.h"
#include "ui/windows/UIPaymentWindow.h"
#include "ui/windows/UIMultiplayerWindow.h"
#include "ui/windows/UICampaignWindow.h"
#include "ui/windows/UICampaignMapWindow.h"
#include "ui/windows/UICreateServerWindow.h"
#include "ui/windows/UIMapOptionsWindow.h"
#include "ui/windows/UISettingsWindow.h"
#include "ui/windows/UIGestureWindow.h"
#include "ui/windows/UIGooglePlayWindow.h"
#include "ui/windows/UIMapFinishedWindow.h"
#include "ui/windows/UIModeSelectionWindow.h"

#include "campaign/ICampaignManager.h"
#include "campaign/persister/GooglePlayPersister.h"

#include "caveexpress/shared/constants/Commands.h"
#include "caveexpress/shared/constants/Commands.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/client/entities/ClientWindowTile.h"
#include "caveexpress/client/entities/ClientCaveTile.h"
#include "caveexpress/client/entities/ClientNPC.h"
#include "caveexpress/client/entities/ClientParticle.h"
#include "caveexpress/client/ui/windows/UIMapEditorWindow.h"
#include "caveexpress/client/ui/nodes/UINodeMapEditor.h"
#include "caveexpress/client/ui/windows/UIMapFailedWindow.h"
#include "caveexpress/client/ui/windows/UIMainWindow.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorWindow.h"
#include "caveexpress/client/ui/windows/UIGameHelpWindow.h"
#include "caveexpress/client/ui/windows/UIGameOverWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorHelpWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorOptionsWindow.h"
#include "caveexpress/client/ui/windows/UIGameFinishedWindow.h"
#include "caveexpress/client/ui/windows/UICaveExpressSettingsWindow.h"
#include "caveexpress/client/ui/windows/intro/IntroPackage.h"
#include "caveexpress/client/ui/windows/intro/IntroTime.h"
#include "caveexpress/client/ui/windows/intro/IntroTree.h"
#include "caveexpress/client/ui/windows/intro/IntroGeyser.h"
#include "caveexpress/client/ui/windows/intro/IntroAttack.h"
#include "caveexpress/client/ui/windows/intro/IntroFlying.h"
#include "caveexpress/client/ui/windows/intro/IntroFindYourWay.h"
#include "caveexpress/client/CaveExpressClientMap.h"
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
#include "caveexpress/client/network/TargetCaveHandler.h"
#include "caveexpress/client/network/AnnounceTargetCaveHandler.h"
#include "caveexpress/client/network/FailedMapHandler.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/server/entities/Fruit.h"
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

#include <SDL_assert.h>

namespace caveexpress {

PROTOCOL_CLASS_FACTORY_IMPL(DropMessage);
PROTOCOL_CLASS_FACTORY_IMPL(RemoveRopeMessage);
PROTOCOL_CLASS_FACTORY_IMPL(AddRopeMessage);
PROTOCOL_CLASS_FACTORY_IMPL(LightStateMessage);
PROTOCOL_CLASS_FACTORY_IMPL(AddCaveMessage);
PROTOCOL_CLASS_FACTORY_IMPL(UpdateCollectedTypeMessage);
PROTOCOL_CLASS_FACTORY_IMPL(WaterHeightMessage);
PROTOCOL_CLASS_FACTORY_IMPL(WaterImpactMessage);
PROTOCOL_CLASS_FACTORY_IMPL(TargetCaveMessage);
PROTOCOL_CLASS_FACTORY_IMPL(AnnounceTargetCaveMessage);

CaveExpress::CaveExpress () :
		_persister(nullptr), _campaignManager(nullptr), _clientMap(nullptr), _updateEntitiesTime(0), _frontend(nullptr), _serviceProvider(nullptr),_connectedClients(
				0), _packageCount(0), _loadDelay(0), _loadDelayName("")
{
}

CaveExpress::~CaveExpress ()
{
	Commands.removeCommand(CMD_MAP_OPEN_IN_EDITOR);
	Commands.removeCommand(CMD_DROP);
	delete _persister;
	delete _campaignManager;
	delete _clientMap;
}

DirectoryEntries CaveExpress::listDirectory(const std::string& basedir, const std::string& subdir) {
	DirectoryEntries entriesAll;
	#include "caveexpress-files.h"
	return entriesAll;
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
		Log::info(LOG_GAMEIMPL, "reset the game state");
		_campaignManager->reset();
		return;
	}

	const bool isDone = _map.isDone();
	if (isDone && !_map.isRestartInitialized()) {
		const uint32_t playTime = _map.getTime();
		const uint32_t referenceTime = _map.getReferenceTime();
		const float relativeRefTime = Config.getConfigVar(REFERENCE_TIME_FACTOR)->getFloatValue() * referenceTime;
		// this max is needed for debug reasons (if you force a map win)
		const float time = std::max(1.0f, playTime / 1000.0f);
		const uint32_t timeSeconds = std::max(1U, playTime / 1000U);
		const float pointsRate = relativeRefTime / time;
		const uint32_t timePoints = pointsRate * _map.getFinishPoints();
		const uint32_t finishPoints = timePoints + _map.getPoints();
		const int percent = time * 100.0f / relativeRefTime;
		Log::info(LOG_GAMEIMPL, "seconds: %.0f, refseconds: %i, rate: %f, refpoints: %i, timePoints: %i, finishPoints: %i, percent: %i",
						time, _map.getReferenceTime(), pointsRate, _map.getFinishPoints(), timePoints, finishPoints, percent);
		_map.sendSound(0, SoundTypes::SOUND_MUSIC_WIN);
		uint8_t stars = 0;
		if (percent <= 70) {
			stars = 3;
		} else if (percent <= 90) {
			stars = 2;
		} else if (percent <= 100) {
			stars = 1;
		}
		const bool tutorial = string::toBool(_map.getSetting(msn::TUTORIAL));
		if (tutorial)
			Config.increaseCounter("mapfinishedcounter");
		if (!_campaignManager->updateMapValues(_map.getName(), finishPoints, timeSeconds, stars))
			Log::error(LOG_GAMEIMPL, "Could not save the values for the map");

		System.track("mapstate", string::format("finished: %s with %i points in %i seconds and with %i stars", _map.getName().c_str(), finishPoints, timeSeconds, stars));
		GameEvent.finishedMap(_map.getName(), finishPoints, timeSeconds, stars);
	} else if (!isDone && _map.isFailed()) {
		Log::debug(LOG_GAMEIMPL, "map failed");
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
	if (_connectedClients == 0) {
		Log::error(LOG_GAMEIMPL, "client counts are out of sync");
	} else {
		_connectedClients--;
	}
	return _connectedClients;
}

int CaveExpress::getPlayers ()
{
	return _map.getConnectedPlayers();
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
	struct {
		const char *configVar;
		const char *value;
		int flags;
	} gameConfigVars[] = {
		{MAX_HITPOINTS, "100", CV_NOPERSIST},
		{DAMAGE_THRESHOLD, "0.3", CV_NOPERSIST},
		{REFERENCE_TIME_FACTOR, "1.0", CV_NOPERSIST},
		{FRUIT_COLLECT_DELAY_FOR_A_NEW_LIFE, "15000", CV_NOPERSIST},
		{AMOUNT_OF_FRUITS_FOR_A_NEW_LIFE, "4", CV_NOPERSIST},
		{FRUIT_HITPOINTS, "10", CV_NOPERSIST},
		{WATER_PARTICLE, "false", CV_READONLY | CV_NOPERSIST},
		{NPC_FLYING_SPEED, "2.0", CV_NOPERSIST}
	};

	const int n = SDL_arraysize(gameConfigVars);
	for (int i = 0; i < n; ++i) {
		Config.initOrGetConfigVar(gameConfigVars[i].configVar, gameConfigVars[i].value, gameConfigVars[i].flags);
	}

	// we have to override this - otherwise the old value from the config is used... which would be bad
	Config.getConfigVar(NPC_FLYING_SPEED)->setValue("2.0");

	ClientEntityRegistry &r = Singleton<ClientEntityRegistry>::getInstance();
	r.registerFactory(&EntityTypes::DECORATION, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::SOLID, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::LAVA, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::GROUND, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::CAVE, ClientCaveTile::FACTORY);
	r.registerFactory(&EntityTypes::WINDOW, ClientWindowTile::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FRIENDLY_GRANDPA, ClientNPC::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FRIENDLY_WOMAN, ClientNPC::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FRIENDLY_MAN, ClientNPC::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FISH, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FLYING, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::NPC_WALKING, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::NPC_MAMMUT, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::NPC_BLOWING, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PLAYER, ClientPlayer::FACTORY);
	r.registerFactory(&EntityTypes::STONE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::TREE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGE_ICE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGE_ROCK, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGETARGET_ICE, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGETARGET_ROCK, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::APPLE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::BANANA, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::EGG, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PARTICLE, ClientParticle::FACTORY);
	r.registerFactory(&EntityTypes::GEYSER_ICE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::GEYSER_ROCK, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::BOMB, ClientEntity::FACTORY);

	ProtocolMessageFactory& f = ProtocolMessageFactory::get();
	f.registerFactory(protocol::PROTO_DROP, DropMessage::FACTORY);
	f.registerFactory(protocol::PROTO_REMOVEROPE, RemoveRopeMessage::FACTORY);
	f.registerFactory(protocol::PROTO_WATERHEIGHT, WaterHeightMessage::FACTORY);
	f.registerFactory(protocol::PROTO_WATERIMPACT, WaterImpactMessage::FACTORY);
	f.registerFactory(protocol::PROTO_ADDCAVE, AddCaveMessage::FACTORY);
	f.registerFactory(protocol::PROTO_LIGHTSTATE, LightStateMessage::FACTORY);
	f.registerFactory(protocol::PROTO_TARGETCAVE, TargetCaveMessage::FACTORY);
	f.registerFactory(protocol::PROTO_ANNOUNCETARGETCAVE, AnnounceTargetCaveMessage::FACTORY);
	f.registerFactory(protocol::PROTO_UPDATECOLLECTEDTYPE,UpdateCollectedTypeMessage::FACTORY);
	f.registerFactory(protocol::PROTO_ADDROPE, AddRopeMessage::FACTORY);

	{
		ExecutionTime e("loading persister");
		const ConfigVarPtr& persister = Config.getConfigVar("persister", "sqlite", true, CV_READONLY);
		if (persister->getValue() == "nop") {
			_persister = new NOPPersister();
		} else if (persister->getValue() == "googleplay" && System.supportGooglePlay()) {
			IGameStatePersister *delegate = new SQLitePersister(System.getDatabaseDirectory() + "gamestate.sqlite");
			_persister = new GooglePlayPersister(delegate);
		} else {
			_persister = new SQLitePersister(System.getDatabaseDirectory() + "gamestate.sqlite");
		}
		if (!_persister->init()) {
			Log::error(LOG_GAMEIMPL, "Failed to initialize the persister");
		}
	}
	{
		ExecutionTime e("campaign manager");
		_campaignManager = new CampaignManager(_persister, serviceProvider.getMapManager());
		_campaignManager->init();
		_campaignManager->addListener(this);
	}
	ProtocolHandlerRegistry& rp = ProtocolHandlerRegistry::get();
	rp.registerServerHandler(::protocol::PROTO_SPAWN, new SpawnHandler(_map, _campaignManager));
	rp.registerServerHandler(::protocol::PROTO_DISCONNECT, new DisconnectHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_STARTMAP, new StartMapHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_MOVEMENT, new MovementHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_FINGERMOVEMENT, new FingerMovementHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_STOPFINGERMOVEMENT, new StopFingerMovementHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_STOPMOVEMENT, new StopMovementHandler(_map));
	rp.registerServerHandler(protocol::PROTO_DROP, new DropHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_ERROR, new ErrorHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_CLIENTINIT, new ClientInitHandler(_map));

	_frontend = frontend;
	_serviceProvider = &serviceProvider;
	_map.init(_frontend, *_serviceProvider);
	GameEvent.init(*_serviceProvider);
}

void CaveExpress::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	SDL_assert(_campaignManager != nullptr);
	UI& ui = UI::get();
	CampaignManager& campaignMgr = *_campaignManager;
	ui.addWindow(new UIMainWindow(frontend, serviceProvider));
	ui.addWindow(new UICampaignWindow(frontend, serviceProvider, campaignMgr));
	ui.addWindow(new UICampaignMapWindow(frontend, campaignMgr));
	CaveExpressClientMap *map = new CaveExpressClientMap(0, 0, frontend->getWidth(), frontend->getHeight(), frontend, serviceProvider, UI::get().loadTexture("tile-reference")->getWidth());
	// if we reinit the ui - we have to destroy previously allocated memory
	delete _clientMap;
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
	UINodeMapEditor* editor = new UINodeMapEditor(frontend, serviceProvider.getMapManager());
	UIMapEditorWindow* mapEditorWindow = new UIMapEditorWindow(frontend, serviceProvider.getMapManager(), editor);
	ui.addWindow(mapEditorWindow);
	ui.addWindow(new UIGameHelpWindow(frontend));
	ui.addWindow(new UIGestureWindow(frontend));
	ui.addWindow(new UIGameOverWindow(frontend, campaignMgr));
	ui.addWindow(new UIMapOptionsWindow(frontend, serviceProvider));
	ui.addWindow(new UIGameFinishedWindow(frontend));
	ui.addWindow(new UIGooglePlayWindow(frontend));
	ui.addWindow(new UIMapFinishedWindow(frontend, campaignMgr, serviceProvider, SoundTypes::SOUND_PACKAGE_COLLIDE));
	ui.addWindow(new UIMapFailedWindow(frontend, campaignMgr));
	ui.addWindow(new UIMapEditorHelpWindow(frontend));
	ui.addWindow(new UIMapEditorOptionsWindow(frontend, editor));

	Commands.registerCommandVoid(CMD_DROP, [=] () { map->drop(); });
	CommandPtr cmd = Commands.registerCommandVoid(CMD_MAP_OPEN_IN_EDITOR, [=] () {
		if (!map->isActive())
			return;

		const std::string& name = map->getName();
		Commands.executeCommandLine(CMD_LOADMAP " " + name);
	});
	cmd->setCompleter([&] (const std::string& input, std::vector<std::string>& matches) {
		for (auto entry : _serviceProvider->getMapManager().getMapsByWildcard(input + "*")) {
			matches.push_back(entry.first);
		}
	});

	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.unregisterClientHandler(protocol::PROTO_ADDROPE);
	r.registerClientHandler(protocol::PROTO_ADDROPE, new AddRopeHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_REMOVEROPE);
	r.registerClientHandler(protocol::PROTO_REMOVEROPE, new RemoveRopeHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_WATERHEIGHT);
	r.registerClientHandler(protocol::PROTO_WATERHEIGHT, new WaterHeightHandler(*map));
	r.unregisterClientHandler(::protocol::PROTO_UPDATEPACKAGECOUNT);
	r.registerClientHandler(::protocol::PROTO_UPDATEPACKAGECOUNT, new UpdatePackageCountHandler());
	r.unregisterClientHandler(protocol::PROTO_UPDATECOLLECTEDTYPE);
	r.registerClientHandler(protocol::PROTO_UPDATECOLLECTEDTYPE, new UpdateCollectedTypeHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_WATERIMPACT);
	r.registerClientHandler(protocol::PROTO_WATERIMPACT, new WaterImpactHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_ADDCAVE);
	r.registerClientHandler(protocol::PROTO_ADDCAVE, new AddCaveHandler(*map));
	r.unregisterClientHandler(protocol::PROTO_LIGHTSTATE);
	r.registerClientHandler(protocol::PROTO_LIGHTSTATE, new LightStateHandler(*map));
	r.unregisterClientHandler(::protocol::PROTO_ADDENTITY);
	r.registerClientHandler(::protocol::PROTO_ADDENTITY, new AddEntityWithSoundHandler(*map));
	r.unregisterClientHandler(::protocol::PROTO_INITDONE);
	r.registerClientHandler(::protocol::PROTO_INITDONE, new HudInitDoneHandler(*map));
	r.unregisterClientHandler(::protocol::PROTO_FAILEDMAP);
	r.registerClientHandler(::protocol::PROTO_FAILEDMAP, new FailedMapHandler(*map, serviceProvider));
	r.unregisterClientHandler(protocol::PROTO_TARGETCAVE);
	r.registerClientHandler(protocol::PROTO_TARGETCAVE, new TargetCaveHandler());
	r.unregisterClientHandler(protocol::PROTO_ANNOUNCETARGETCAVE);
	r.registerClientHandler(protocol::PROTO_ANNOUNCETARGETCAVE, new AnnounceTargetCaveHandler(*map));
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

}

static GameRegisterStatic CAVEEXPRESS("caveexpress", GamePtr(new caveexpress::CaveExpress()));
