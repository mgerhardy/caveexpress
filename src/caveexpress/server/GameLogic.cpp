#include "GameLogic.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/server/entities/Fruit.h"
#include "engine/common/campaign/ICampaignManager.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Shared.h"
#include "engine/common/Logger.h"
#include "engine/common/CommandSystem.h"
#include "caveexpress/shared/constants/Commands.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "engine/common/SQLite.h"
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
#include "engine/common/network/messages/LoadMapMessage.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"

GameLogic::GameLogic () :
		_updateEntitiesTime(0), _frontend(nullptr), _serviceProvider(nullptr), _campaignManager(nullptr), _connectedClients(
				0), _packageCount(0), _loadDelay(0), _loadDelayName("")
{
}

GameLogic::~GameLogic ()
{
}

void GameLogic::init (IFrontend *frontend, ServiceProvider *serviceProvider, ICampaignManager *campaignManager)
{
	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerServerHandler(protocol::PROTO_SPAWN, new SpawnHandler(_map, campaignManager));
	r.registerServerHandler(protocol::PROTO_DISCONNECT, new DisconnectHandler(_map));
	r.registerServerHandler(protocol::PROTO_STARTMAP, new StartMapHandler(_map));
	r.registerServerHandler(protocol::PROTO_MOVEMENT, new MovementHandler(_map));
	r.registerServerHandler(protocol::PROTO_FINGERMOVEMENT, new FingerMovementHandler(_map));
	r.registerServerHandler(protocol::PROTO_STOPFINGERMOVEMENT, new StopFingerMovementHandler(_map));
	r.registerServerHandler(protocol::PROTO_STOPMOVEMENT, new StopMovementHandler(_map));
	r.registerServerHandler(protocol::PROTO_DROP, new DropHandler(_map));
	r.registerServerHandler(protocol::PROTO_ERROR, new ErrorHandler(_map));
	r.registerServerHandler(protocol::PROTO_CLIENTINIT, new ClientInitHandler(_map));
	_frontend = frontend;
	_serviceProvider = serviceProvider;
	_campaignManager = campaignManager;
	_map.init(_frontend, *_serviceProvider);
	GameEvent.init(*_serviceProvider);
}

void GameLogic::shutdown ()
{
	_map.shutdown();
}

void GameLogic::loadDelayed (const std::string& name)
{
	_loadDelay = 3000;
	_loadDelayName = name;
	GameEvent.restartMap(_loadDelay);
}

bool GameLogic::update (uint32_t deltaTime)
{
	if (!_map.isActive() || deltaTime == 0)
		return false;

	if (!_serviceProvider->getNetwork().isServer()) {
		_map.resetCurrentMap();
		return false;
	}

	if (_map.isPause())
		return false;

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
			loadMap(_loadDelayName);
			return true;
		}

		return false;
	}

	if (_map.handleDeadPlayers() > 0 && !_map.isActive()) {
		info(LOG_SERVER, "reset the game state");
		_campaignManager->reset();
		return false;
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
		return true;
	} else if (!isDone && _map.isFailed()) {
		debug(LOG_SERVER, "map failed");
		const uint32_t delay = 1000;
		_map.restart(delay);
		return false;
	}

	return false;
}

bool GameLogic::visitEntity (IEntity *entity)
{
	if (!entity->isDynamic() && !entity->isWater() && !entity->isCave())
		return false;

	if (entity->isDirty()) {
		entity->snapshot();
		GameEvent.updateEntity(entity->getVisMask(), *entity);
	}
	return false;
}

void GameLogic::onData (ClientId clientId, ByteStream &data)
{
	while (!data.empty()) {
		const ScopedPtr<IProtocolMessage> msg(ProtocolMessageFactory::get().create(data));
		if (!msg) {
			error(LOG_SERVER, "no message for type " + string::toString(static_cast<int>(data.readByte())));
			continue;
		}
		IServerProtocolHandler* handler = ProtocolHandlerRegistry::get().getServerHandler(*msg);
		if (handler == nullptr) {
			error(LOG_SERVER, String::format("no server handler for message type %i", msg->getId()));
			continue;
		}
		handler->execute(clientId, *msg);
	}
}

int GameLogic::disconnect (ClientId clientId)
{
	_map.removePlayer(clientId);
	_connectedClients--;
	if (_connectedClients < 0) {
		_connectedClients = 0;
		error(LOG_SERVER, "client counts are out of sync");
	}

	return _connectedClients;
}

Map& GameLogic::getMap ()
{
	return _map;
}

void GameLogic::connect (ClientId clientId)
{
	_connectedClients++;
	const LoadMapMessage msg(_map.getName(), _map.getTitle());
	_serviceProvider->getNetwork().sendToClients(ClientIdToClientMask(clientId), msg);
}

bool GameLogic::loadMap (const std::string& mapName)
{
	_loadDelay = 0;
	_updateEntitiesTime = 0;
	_packageCount = 0;
	return _map.load(mapName);
}
