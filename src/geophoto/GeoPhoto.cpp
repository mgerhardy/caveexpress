#include "GeoPhoto.h"
#include "engine/client/ui/UI.h"
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

uint8_t GeoPhoto::getStars () const {
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

bool GeoPhoto::mapLoad (const std::string& map)
{
	return _map.load(map);
}

void GeoPhoto::mapReload ()
{
	_map.reload();
}

void GeoPhoto::mapShutdown ()
{
	_map.shutdown();
}

void GeoPhoto::connect (ClientId clientId)
{
	const LoadMapMessage msg(_map.getName(), _map.getTitle());
	_serviceProvider->getNetwork().sendToClients(ClientIdToClientMask(clientId), msg);
}

int GeoPhoto::disconnect (ClientId clientId)
{
	_map.removePlayer(clientId);
	return 0;
}

int GeoPhoto::getPlayers ()
{
	return _map.getPlayers().size();
}

std::string GeoPhoto::getMapName ()
{
	return _map.getName();
}

void GeoPhoto::shutdown ()
{
	mapShutdown();
}

void GeoPhoto::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;

	_roundController = new RoundController(serviceProvider.getGameStatePersister(), &UI::get());

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

void GeoPhoto::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	UI& ui = UI::get();

	ui.addWindow(new UIMainWindow(&frontend));
	ui.addWindow(new UISettingsWindow(&frontend));
	ui.addWindow(new UIMapWindow(&frontend, *_roundController));
	ui.addWindow(new UIRoundResultWindow(&frontend, _roundController->getGameRound()));
	ui.addWindow(new UIViewResultWindow(&frontend, _roundController->getGameRound()));
}

bool GeoPhoto::visitEntity (IEntity *entity)
{
	if (entity->isDynamic()) {
		_map.updateEntity(0, *entity);
	}
	return false;
}
