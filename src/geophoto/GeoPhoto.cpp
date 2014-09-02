#include "GeoPhoto.h"
#include "engine/client/ui/UI.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/campaign/persister/SQLitePersister.h"
#include "geophoto/client/ui/windows/UIMainWindow.h"
#include "geophoto/client/ui/windows/UIMapWindow.h"
#include "geophoto/client/ui/windows/UISettingsWindow.h"
#include "geophoto/client/ui/windows/UIRoundResultWindow.h"
#include "geophoto/client/ui/windows/UIViewResultWindow.h"
#include "geophoto/client/round/RoundController.h"
#include <SDL.h>

GeoPhoto::GeoPhoto ():
	_persister(nullptr), _campaignManager(nullptr), _frontend(nullptr), _serviceProvider(nullptr)
{
	delete _persister;
	delete _campaignManager;
	delete _roundController;
}

GeoPhoto::~GeoPhoto ()
{
}

IMapManager* GeoPhoto::getMapManager ()
{
	return new FileMapManager("sok");
}

void GeoPhoto::update (uint32_t deltaTime)
{
}

uint8_t GeoPhoto::getStars () const {
	return 0;
}

bool GeoPhoto::mapLoad (const std::string& map)
{
	return false;
}

void GeoPhoto::mapReload ()
{
}

void GeoPhoto::mapShutdown ()
{
}

void GeoPhoto::connect (ClientId clientId)
{
}

int GeoPhoto::disconnect (ClientId clientId)
{
	return 0;
}

int GeoPhoto::getPlayers ()
{
	return 1;
}

std::string GeoPhoto::getMapName ()
{
	return "";
}

void GeoPhoto::shutdown ()
{
	mapShutdown();
}

void GeoPhoto::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;

	IGameStatePersister* gamePersister = new NOPPersister(); // TODO:
	_roundController = new RoundController(*gamePersister, &UI::get());

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

#if 0
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
#endif
}

void GeoPhoto::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	UI& ui = UI::get();

	ui.addWindow(new UIMainWindow(frontend));
	ui.addWindow(new UISettingsWindow(frontend));
	ui.addWindow(new UIMapWindow(frontend, *_roundController));
	ui.addWindow(new UIRoundResultWindow(frontend, _roundController->getGameRound()));
	ui.addWindow(new UIViewResultWindow(frontend, _roundController->getGameRound()));
}

bool GeoPhoto::visitEntity (IEntity *entity)
{
	return false;
}
