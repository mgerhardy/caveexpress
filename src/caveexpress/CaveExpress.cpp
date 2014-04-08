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
#include "caveexpress/client/entities/ClientWindowTile.h"
#include "caveexpress/client/entities/ClientCaveTile.h"
#include "caveexpress/client/entities/ClientParticle.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/client/commands/CmdDrop.h"
#include "caveexpress/shared/constants/Commands.h"
#include "caveexpress/client/ui/windows/UIMainWindow.h"
#include "caveexpress/client/ui/windows/UICampaignWindow.h"
#include "caveexpress/client/ui/windows/UICampaignMapWindow.h"
#include "caveexpress/client/ui/windows/UICreateServerWindow.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "caveexpress/client/ui/windows/UIMultiplayerWindow.h"
#include "caveexpress/client/ui/windows/UIMapOptionsWindow.h"
#include "caveexpress/client/ui/windows/UISettingsWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorWindow.h"
#include "caveexpress/client/ui/windows/UIGameHelpWindow.h"
#include "caveexpress/client/ui/windows/UIGameOverWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorHelpWindow.h"
#include "caveexpress/client/ui/windows/UIMapEditorOptionsWindow.h"
#include "caveexpress/client/ui/windows/UIGameFinishedWindow.h"
#include "caveexpress/client/ui/windows/UIMapFinishedWindow.h"
#include "caveexpress/client/ui/windows/UIMapFailedWindow.h"
#include "caveexpress/client/ui/windows/UIPaymentWindow.h"
#include "caveexpress/client/ui/windows/UIPopupWindow.h"
#include "caveexpress/client/ui/windows/UIModeSelectionWindow.h"
#include "caveexpress/client/ui/windows/intro/Intro1.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/client/CaveExpressClientMap.h"
#include "caveexpress/client/commands/CmdMapOpenInEditor.h"
#include "caveexpress/client/network/HudLoadMapHandler.h"
#include "caveexpress/client/network/AddRopeHandler.h"
#include "caveexpress/client/network/RemoveRopeHandler.h"
#include "caveexpress/client/network/AddEntityWithSoundHandler.h"
#include "caveexpress/client/network/HudMapSettingsHandler.h"
#include "caveexpress/client/network/WaterHeightHandler.h"
#include "caveexpress/client/network/CloseMapHandler.h"
#include "caveexpress/client/network/StartMapHandler.h"
#include "caveexpress/client/network/UpdateHitpointsHandler.h"
#include "caveexpress/client/network/UpdateLivesHandler.h"
#include "caveexpress/client/network/UpdateHitpointsHandler.h"
#include "caveexpress/client/network/UpdateCollectedTypeHandler.h"
#include "caveexpress/client/network/PlayerListHandler.h"
#include "caveexpress/client/network/SpawnInfoHandler.h"
#include "caveexpress/client/network/TextMessageHandler.h"
#include "caveexpress/client/network/UpdatePackageCountHandler.h"
#include "caveexpress/client/network/WaterImpactHandler.h"
#include "caveexpress/client/network/AddCaveHandler.h"
#include "caveexpress/client/network/LightStateHandler.h"
#include "caveexpress/client/network/HudInitDoneHandler.h"
#include "caveexpress/client/network/InitWaitingMapHandler.h"
#include "caveexpress/client/network/UpdatePointsHandler.h"
#include "caveexpress/client/network/TimeRemainingHandler.h"
#include "caveexpress/client/network/FinishedMapHandler.h"
#include "caveexpress/client/network/FailedMapHandler.h"
#include "caveexpress/client/network/UpdateParticleHandler.h"

CaveExpress::CaveExpress () :
		_persister(nullptr), _campaignManager(nullptr), _map(nullptr)
{
}

CaveExpress::~CaveExpress ()
{
	if (_persister != nullptr)
		delete _persister;
	if (_campaignManager != nullptr)
		delete _campaignManager;
}

void CaveExpress::initSoundCache ()
{
	// TODO: async
	ExecutionTime timeCache("Sound cache");
	for (SoundType::TypeMapConstIter i = SoundType::begin(); i != SoundType::end(); ++i) {
		SoundControl.cache(i->second->getSound());
	}
}

void CaveExpress::connect ()
{
	const std::string command = CMD_CL_CONNECT " localhost " + string::toString(Config.getPort());
	Commands.executeCommandLine(command);
}

void CaveExpress::update (uint32_t deltaTime)
{
	_game.update(deltaTime);
}

void CaveExpress::onData (ClientId clientId, ByteStream &data)
{
	_game.onData(clientId, data);
}

bool CaveExpress::mapLoad (const std::string& map)
{
	return _game.loadMap(map);
}

void CaveExpress::mapReload ()
{
	_game.getMap().reload();
}

void CaveExpress::mapShutdown ()
{
	_game.shutdown();
}

void CaveExpress::connect (ClientId clientId)
{
	_game.connect(clientId);
}

int CaveExpress::disconnect (ClientId clientId)
{
	return _game.disconnect(clientId);
}

int CaveExpress::getPlayers ()
{
	return _game.getMap().getPlayers().size();
}

UIWindow* CaveExpress::createPopupWindow (IFrontend* frontend, const std::string& text, int flags, UIPopupCallbackPtr callback)
{
	return new UIPopupWindow(frontend, text, flags, callback);
}

std::string CaveExpress::getMapName ()
{
	return _game.getMap().getName();
}

void CaveExpress::shutdown ()
{
	_game.shutdown();
}

void CaveExpress::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	ClientEntityRegistry &r = Singleton<ClientEntityRegistry>::getInstance();
	r.registerFactory(EntityTypes::DECORATION, ClientMapTile::FACTORY);
	r.registerFactory(EntityTypes::SOLID, ClientMapTile::FACTORY);
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
	_game.init(frontend, &serviceProvider, *_campaignManager);
}

void CaveExpress::initUI (IFrontend* _frontend, ServiceProvider& serviceProvider)
{
	UI& ui = UI::get();
	CampaignManager& campaignMgr = *_campaignManager;
	ui.addWindow(new UIMainWindow(_frontend, serviceProvider));
	ui.addWindow(new UICampaignWindow(_frontend, serviceProvider, campaignMgr));
	ui.addWindow(new UICampaignMapWindow(_frontend, campaignMgr));
	CaveExpressClientMap *map = new CaveExpressClientMap(0, 0, _frontend->getWidth(), _frontend->getHeight(), _frontend, serviceProvider, UI::get().loadTexture("tile-reference")->getWidth());
	_map = map;
	UIMapWindow *mapWindow = new UIMapWindow(_frontend, serviceProvider, campaignMgr, *_map);
	ui.addWindow(mapWindow);
	ui.addWindow(new UIModeSelectionWindow(_frontend, campaignMgr));
	ui.addWindow(new UISettingsWindow(_frontend, serviceProvider, campaignMgr));
	ui.addWindow(new Intro1(_frontend));
	ui.addWindow(new UIPaymentWindow(_frontend));
	ui.addWindow(new UIMultiplayerWindow(_frontend, serviceProvider.getMapManager(), serviceProvider));
	ui.addWindow(new UICreateServerWindow(_frontend, serviceProvider.getMapManager()));
	UIMapEditorWindow* mapEditorWindow = new UIMapEditorWindow(_frontend, serviceProvider.getMapManager());
	ui.addWindow(mapEditorWindow);
	ui.addWindow(new UIGameHelpWindow(_frontend));
	ui.addWindow(new UIGameOverWindow(_frontend, campaignMgr));
	ui.addWindow(new UIMapOptionsWindow(_frontend, serviceProvider));
	ui.addWindow(new UIGameFinishedWindow(_frontend));
	ui.addWindow(new UIMapFinishedWindow(_frontend, campaignMgr, serviceProvider));
	ui.addWindow(new UIMapFailedWindow(_frontend, campaignMgr));
	ui.addWindow(new UIMapEditorHelpWindow(_frontend));
	ui.addWindow(new UIMapEditorOptionsWindow(_frontend, mapEditorWindow->getMapEditorNode()));

	Commands.registerCommand(CMD_DROP, new CmdDrop(*map));
	Commands.registerCommand(CMD_MAP_OPEN_IN_EDITOR, new CmdMapOpenInEditor(*_map));

	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerClientHandler(protocol::PROTO_ADDROPE, new AddRopeHandler(*_map));
	r.registerClientHandler(protocol::PROTO_REMOVEROPE, new RemoveRopeHandler(*_map));
	r.registerClientHandler(protocol::PROTO_WATERHEIGHT, new WaterHeightHandler(*map));
	r.registerClientHandler(protocol::PROTO_UPDATEHITPOINTS, new UpdateHitpointsHandler());
	r.registerClientHandler(protocol::PROTO_UPDATELIVES, new UpdateLivesHandler(*_campaignManager));
	r.registerClientHandler(protocol::PROTO_UPDATEPOINTS, new UpdatePointsHandler());
	r.registerClientHandler(protocol::PROTO_UPDATEPACKAGECOUNT, new UpdatePackageCountHandler());
	r.registerClientHandler(protocol::PROTO_UPDATECOLLECTEDTYPE, new UpdateCollectedTypeHandler(*_map));
	r.registerClientHandler(protocol::PROTO_TIMEREMAINING, new TimeRemainingHandler());
	r.registerClientHandler(protocol::PROTO_WATERIMPACT, new WaterImpactHandler(*map));
	r.registerClientHandler(protocol::PROTO_ADDCAVE, new AddCaveHandler(*map));
	r.registerClientHandler(protocol::PROTO_LIGHTSTATE, new LightStateHandler(*map));
	r.registerClientHandler(protocol::PROTO_ADDENTITY, new AddEntityWithSoundHandler(*_map));
	r.registerClientHandler(protocol::PROTO_LOADMAP, new HudLoadMapHandler(*_map, serviceProvider));
	r.registerClientHandler(protocol::PROTO_MAPSETTINGS, new HudMapSettingsHandler(*_map));
	r.registerClientHandler(protocol::PROTO_INITDONE, new HudInitDoneHandler(*_map));
}
