#include "MiniRacer.h"
#include "ui/UI.h"
#include "miniracer/client/ui/windows/UIMainWindow.h"
#include "miniracer/client/ui/windows/UIMapWindow.h"
#include "ui/windows/IUIMapEditorWindow.h"
#include "ui/windows/UIMapEditorHelpWindow.h"
#include "miniracer/client/MiniRacerClientMap.h"
#include "miniracer/client/ui/nodes/UINodeMapEditor.h"
#include "miniracer/client/ui/nodes/UINodeSpriteSelector.h"
#include "miniracer/client/ui/nodes/UINodeEntitySelector.h"
#include "miniracer/server/network/SpawnHandler.h"
#include "miniracer/server/network/DisconnectHandler.h"
#include "miniracer/server/network/StartMapHandler.h"
#include "miniracer/server/network/MovementHandler.h"
#include "miniracer/server/network/StopMovementHandler.h"
#include "miniracer/server/network/ClientInitHandler.h"
#include "miniracer/server/network/ErrorHandler.h"
#include "miniracer/server/network/StopFingerMovementHandler.h"
#include "miniracer/server/network/FingerMovementHandler.h"
#include "miniracer/shared/MiniRacerEntityType.h"
#include "miniracer/shared/MiniRacerAchievement.h"
#include "miniracer/shared/network/ProtocolMessageTypes.h"
#include "miniracer/shared/network/messages/ProtocolMessages.h"
#include "miniracer/shared/constants/Commands.h"
#include "miniracer/client/commands/CmdMapOpenInEditor.h"
#include "client/entities/ClientEntityFactory.h"
#include "client/entities/ClientMapTile.h"
#include "network/ProtocolHandlerRegistry.h"
#include "campaign/ICampaignManager.h"
#include "campaign/persister/GooglePlayPersister.h"
#include "common/ConfigManager.h"
#include "service/ServiceProvider.h"
#include "common/System.h"
#include "common/ExecutionTime.h"
#include "common/Commands.h"
#include "common/CommandSystem.h"
#include "network/INetwork.h"
#include "network/messages/LoadMapMessage.h"
#include "network/messages/FinishedMapMessage.h"
#include "ui/windows/UICampaignWindow.h"
#include "ui/windows/UICampaignMapWindow.h"
#include "miniracer/client/ui/windows/UIMiniRacerMapOptionsWindow.h"
#include "ui/windows/UIPaymentWindow.h"
#include "ui/windows/UIGooglePlayWindow.h"
#include "ui/windows/UISettingsWindow.h"
#include "ui/windows/UIMapFinishedWindow.h"
#include "ui/windows/UIGestureWindow.h"
#include "ui/windows/UICreateServerWindow.h"
#include "ui/windows/UIMultiplayerWindow.h"
#include "ui/windows/IUIMapEditorOptionsWindow.h"
#include "miniracer/shared/MiniRacerSQLitePersister.h"
#include <SDL.h>

namespace miniracer {

MiniRacer::MiniRacer() :
		_persister(nullptr), _campaignManager(nullptr), _clientMap(nullptr), _frontend(
				nullptr), _serviceProvider(nullptr) {
}

MiniRacer::~MiniRacer() {
	delete _persister;
	delete _campaignManager;
	delete _clientMap;
}

IMapManager* MiniRacer::getMapManager ()
{
	return new LUAMapManager();
}

void MiniRacer::update (uint32_t deltaTime)
{
	_map.autoStart();
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
		_campaignManager->getAutoActiveCampaign();

		const FinishedMapMessage msg(_map.getName(), 0, 0, 0);
		_serviceProvider->getNetwork().sendToAllClients(msg);
	} else if (!isDone && _map.isFailed()) {
		Log::debug(LOG_SERVER, "map failed");
		const uint32_t delay = 1000;
		_map.restart(delay);
	}
}

bool MiniRacer::mapLoad (const std::string& map)
{
	return _map.load(map);
}

void MiniRacer::mapReload ()
{
	_map.reload();
}

void MiniRacer::mapShutdown ()
{
	_map.shutdown();
}

void MiniRacer::connect (ClientId clientId)
{
	const LoadMapMessage msg(_map.getName(), _map.getTitle());
	_serviceProvider->getNetwork().sendToClients(ClientIdToClientMask(clientId), msg);
}

int MiniRacer::disconnect (ClientId clientId)
{
	_map.removePlayer(clientId);
	return 0;
}

int MiniRacer::getPlayers ()
{
	return _map.getConnectedPlayers();
}

std::string MiniRacer::getMapName ()
{
	return _map.getName();
}

int MiniRacer::getMaxClients ()
{
	return _map.getMaxPlayers();
}

void MiniRacer::shutdown ()
{
	mapShutdown();
}

void MiniRacer::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;

	{
		ExecutionTime e("loading persister");
		const ConfigVarPtr& persister = Config.get().getConfigVar("persister", "sqlite", true, CV_READONLY);
		if (persister->getValue() == "nop") {
			_persister = new NOPPersister();
		} else if (persister->getValue() == "googleplay" && System.supportGooglePlay()) {
			IGameStatePersister *delegate = new MiniRacerSQLitePersister(System.getDatabaseDirectory() + "gamestate.sqlite");
			_persister = new GooglePlayPersister(delegate);
		} else {
			_persister = new MiniRacerSQLitePersister(System.getDatabaseDirectory() + "gamestate.sqlite");
		}
		if (!_persister->init()) {
			Log::error(LOG_SERVER, "Failed to initialize the persister");
		}
	}
	{
		ExecutionTime e("campaign manager");
		_campaignManager = new CampaignManager(_persister, serviceProvider.getMapManager());
		_campaignManager->init();
		_campaignManager->addListener(this);
	}

	_map.init(_frontend, *_serviceProvider);

	ClientEntityRegistry &r = Singleton<ClientEntityRegistry>::getInstance();
	r.registerFactory(&EntityTypes::SOLID, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::PLAYER, ClientEntity::FACTORY);

	ProtocolHandlerRegistry& rp = ProtocolHandlerRegistry::get();
	rp.registerServerHandler(::protocol::PROTO_SPAWN, new SpawnHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_DISCONNECT, new DisconnectHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_STARTMAP, new StartMapHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_MOVEMENT, new MovementHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_FINGERMOVEMENT, new FingerMovementHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_STOPFINGERMOVEMENT, new StopFingerMovementHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_STOPMOVEMENT, new StopMovementHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_ERROR, new ErrorHandler(_map));
	rp.registerServerHandler(::protocol::PROTO_CLIENTINIT, new ClientInitHandler(_map));

	_campaignManager->getAutoActiveCampaign();
}

void MiniRacer::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	Log::info(LOG_CLIENT, "Init miniracer ui");
	UI& ui = UI::get();
	ui.disableRotatingFonts();
	ui.addWindow(new UIMainWindow(frontend));
	MiniRacerClientMap *map = new MiniRacerClientMap(0, 0, frontend->getWidth(), frontend->getHeight(), frontend, serviceProvider, UI::get().loadTexture("tile-reference")->getWidth());
	_clientMap = map;
	ui.addWindow(new UIMapWindow(frontend, serviceProvider, *_campaignManager, *map));
	ui.addWindow(new UICampaignMapWindow(frontend, *_campaignManager));
	ui.addWindow(new UIPaymentWindow(frontend));
	ui.addWindow(new UIGooglePlayWindow(frontend));
	UISettingsWindow* settings = new UISettingsWindow(frontend, serviceProvider);
	settings->init();
	ui.addWindow(settings);
	ui.addWindow(new UICampaignWindow(frontend, serviceProvider, *_campaignManager));
	ui.addWindow(new UIGestureWindow(frontend));
	ui.addWindow(new UIMapFinishedWindow(frontend, *_campaignManager, serviceProvider, SoundType::NONE));
	ui.addWindow(new UIMiniRacerMapOptionsWindow(frontend, serviceProvider));
	ui.addWindow(new UIMultiplayerWindow(frontend, serviceProvider.getMapManager(), serviceProvider));
	ui.addWindow(new UICreateServerWindow(frontend, serviceProvider.getMapManager()));

	UINodeMapEditor* editor = new UINodeMapEditor(frontend, serviceProvider.getMapManager());
	UINodeSpriteSelector* spriteSelector = new UINodeSpriteSelector(frontend);
	UINodeEntitySelector* entitySelector = new UINodeEntitySelector(frontend);
	IUIMapEditorWindow* mapEditorWindow = new IUIMapEditorWindow(frontend, serviceProvider.getMapManager(), editor, spriteSelector, entitySelector);
	ui.addWindow(mapEditorWindow);
	ui.addWindow(new UIMapEditorHelpWindow(frontend));
	ui.addWindow(new IUIMapEditorOptionsWindow(frontend, mapEditorWindow->getMapEditorNode()));

	Commands.registerCommand(CMD_MAP_OPEN_IN_EDITOR, new CmdMapOpenInEditor(*map));
}

bool MiniRacer::visitEntity (IEntity *entity)
{
	if (entity->isDynamic()) {
		_map.updateEntity(0, *entity);
	}
	return false;
}

}

static GameRegisterStatic MiniRacer("miniracer", GamePtr(new miniracer::MiniRacer()));
