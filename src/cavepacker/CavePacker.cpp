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
#include "cavepacker/shared/CavePackerAchievement.h"
#include "engine/client/entities/ClientEntityFactory.h"
#include "engine/client/entities/ClientMapTile.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/common/campaign/ICampaignManager.h"
#include "engine/common/campaign/persister/GooglePlayPersister.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/System.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/Commands.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/Achievement.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/network/messages/LoadMapMessage.h"
#include "engine/common/network/messages/FinishedMapMessage.h"
#include "engine/client/ui/windows/UICampaignWindow.h"
#include "engine/client/ui/windows/UICampaignMapWindow.h"
#include "cavepacker/client/ui/windows/UICavePackerMapOptionsWindow.h"
#include "engine/client/ui/windows/UIPaymentWindow.h"
#include "engine/client/ui/windows/UIGooglePlayWindow.h"
#include "engine/client/ui/windows/UISettingsWindow.h"
#include "engine/client/ui/windows/UIMapFinishedWindow.h"
#include "engine/client/ui/windows/UIGestureWindow.h"
#include "cavepacker/shared/CavePackerSQLitePersister.h"
#include <SDL.h>

namespace {
Achievement* puzzleAchievements[] = {
		&Achievements::PUZZLES_10,
		&Achievements::PUZZLES_20,
		&Achievements::PUZZLES_30,
		&Achievements::PUZZLES_40,
		&Achievements::PUZZLES_50,
		&Achievements::PUZZLES_60,
		&Achievements::PUZZLES_70,
		&Achievements::PUZZLES_80,
		&Achievements::PUZZLES_90,
		&Achievements::PUZZLES_100
};
Achievement* fullStarsAchievements[] = {
		&Achievements::STARS_3,
		&Achievements::STARS_10,
		&Achievements::STARS_300
};
}

CavePacker::CavePacker ():
	_persister(nullptr), _campaignManager(nullptr), _clientMap(nullptr), _frontend(nullptr), _serviceProvider(nullptr)
{
}

CavePacker::~CavePacker ()
{
	delete _persister;
	delete _campaignManager;
	delete _clientMap;
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
		const uint32_t moves = _map.getMoves();
		const uint32_t pushes = _map.getPushes();
		const uint8_t stars = getStars();
		_campaignManager->getAutoActiveCampaign();
		if (!_campaignManager->updateMapValues(_map.getName(), moves, pushes, stars, true))
			error(LOG_SERVER, "Could not save the values for the map");

		if (stars == 3) {
			const int n = SDL_arraysize(fullStarsAchievements);
			for (int i = 0; i < n; ++i) {
				fullStarsAchievements[i]->unlock();
			}
		}

		if (_map.getPlayers().size() == 1) {
			const Player* player = _map.getPlayers()[0];
			const std::string& solution = player->getSolution();
			info(LOG_SERVER, "solution: " + solution);
			SDL_SetClipboardText(solution.c_str());
#if 0
			FilePtr solutionFilePtr = FS.getFile(_map.getName() + ".sol");
			if (!solutionFilePtr->exists()) {
				FS.writeFile(solutionFilePtr->getName(), reinterpret_cast<const uint8_t*>(solution.c_str()), solution.size(), true);
			}
#endif
			if (!_map.isAutoSolve()) {
				const std::string solutionId = "solution" + _map.getName();
				System.track(solutionId, solution);
				const int n = SDL_arraysize(puzzleAchievements);
				for (int i = 0; i < n; ++i) {
					puzzleAchievements[i]->unlock();
				}
			} else {
				System.track("autosolve", _map.getName());
			}
			if (!_campaignManager->addAdditionMapData(_map.getName(), solution))
				error(LOG_SERVER, "Could not save the solution for the map");
		} else {
			info(LOG_SERVER, "no solution in multiplayer games");
		}

		System.track("mapstate", String::format("finished: %s with %i moves and %i pushes - got %i stars", _map.getName().c_str(), moves, pushes, stars));
		_map.abortAutoSolve();
		const FinishedMapMessage msg(_map.getName(), moves, pushes, stars);
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
		if (persister->getValue() == "nop") {
			_persister = new NOPPersister();
		} else if (persister->getValue() == "googleplay" && System.supportGooglePlay()) {
			IGameStatePersister *delegate = new CavePackerSQLitePersister(System.getDatabaseDirectory() + "gamestate.sqlite");
			_persister = new GooglePlayPersister(delegate);
		} else {
			_persister = new CavePackerSQLitePersister(System.getDatabaseDirectory() + "gamestate.sqlite");
		}
		if (!_persister->init()) {
			error(LOG_SERVER, "Failed to initialize the persister");
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
	ui.addWindow(new UIGooglePlayWindow(frontend));
	UISettingsWindow* settings = new UISettingsWindow(frontend, serviceProvider);
	settings->init();
	ui.addWindow(settings);
	ui.addWindow(new UICampaignWindow(frontend, serviceProvider, *_campaignManager));
	ui.addWindow(new UIGestureWindow(frontend));
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
