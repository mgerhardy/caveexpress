#include "Map.h"
#include "common/ConfigManager.h"
#include "common/MapSettings.h"
#include "common/Shared.h"
#include "common/Log.h"
#include "service/ServiceProvider.h"
#include "common/SpriteDefinition.h"
#include "common/IFrontend.h"
#include "common/Math.h"
#include "network/INetwork.h"
#include "common/IMapContext.h"
#include "network/messages/InitDoneMessage.h"
#include "network/messages/SoundMessage.h"
#include "network/messages/MapSettingsMessage.h"
#include "network/messages/TextMessage.h"
#include "network/messages/SpawnInfoMessage.h"
#include "network/messages/LoadMapMessage.h"
#include "network/messages/AddEntityMessage.h"
#include "network/messages/RemoveEntityMessage.h"
#include "network/messages/UpdateEntityMessage.h"
#include "network/messages/MapRestartMessage.h"
#include "network/messages/UpdatePointsMessage.h"
#include "network/messages/PauseMessage.h"
#include "common/CommandSystem.h"
#include "common/FileSystem.h"
#include "common/System.h"
#include "common/vec2.h"
#include "common/ExecutionTime.h"
#include "common/Commands.h"
#include "miniracer/server/map/MiniRacerMapContext.h"
#include "miniracer/shared/MiniRacerSpriteType.h"
#include "miniracer/shared/MiniRacerEntityType.h"
#include "miniracer/shared/EntityStates.h"
#include "miniracer/shared/network/messages/ProtocolMessages.h"
#include <SDL.h>
#include <algorithm>
#include <functional>
#include <cassert>
#include <climits>

namespace miniracer {

#define INDEX(col, row) ((col) + _width * (row))

Map::Map () :
		IMap(), _frontend(nullptr), _serviceProvider(nullptr)
{
	Commands.registerCommand(CMD_MAP_PAUSE, bindFunction(Map, triggerPause));
	Commands.registerCommand(CMD_MAP_RESTART, bindFunction(Map, triggerRestart));
	Commands.registerCommand(CMD_START, bindFunction(Map, startMap));

	resetCurrentMap();
}

Map::~Map ()
{
	Commands.removeCommand(CMD_MAP_PAUSE);
	Commands.removeCommand(CMD_MAP_RESTART);
	Commands.removeCommand(CMD_START);
}

void Map::shutdown ()
{
	resetCurrentMap();
}

void Map::sendSound (int clientMask, const SoundType& type, const b2Vec2& pos) const
{
	const SoundMessage msg(pos.x, pos.y, type);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::disconnect (ClientId clientId)
{
	removePlayer(clientId);

	_serviceProvider->getNetwork().disconnectClientFromServer(clientId);

	if (_players.size() == 1 && _playersWaitingForSpawn.empty())
		resetCurrentMap();
}

void Map::triggerRestart ()
{
	if (!_serviceProvider->getNetwork().isServer())
		return;

	Log::info(LOG_MAP, "trigger restart");
	Commands.executeCommandLine(CMD_MAP_START " " + getName());
}

inline bool Map::isActive () const
{
	const bool noEntities = _entities.empty();
	if (noEntities)
		return false;
	const bool noPlayers = _players.empty();
	if (noPlayers)
		return false;
	return true;
}

Player* Map::getPlayer (ClientId clientId)
{
	for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
		if ((*i)->getClientId() == clientId) {
			return *i;
		}
	}

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		if ((*i)->getClientId() == clientId) {
			return *i;
		}
	}

	Log::error(LOG_MAP, "no player found for the client id %i", clientId);
	return nullptr;
}

bool Map::isDone () const
{
	return false;
}

bool Map::isFailed () const
{
	if (_players.empty())
		return true;

	return false;
}

void Map::restart (uint32_t delay)
{
	if (_restartDue > 0)
		return;

	Log::info(LOG_MAP, "trigger map restart");
	_restartDue = _time + delay;
	const MapRestartMessage msg(delay);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void Map::resetCurrentMap ()
{
	_timeManager.reset();
	if (!_name.empty()) {
		const CloseMapMessage msg;
		_serviceProvider->getNetwork().sendToAllClients(msg);
		Log::info(LOG_MAP, "reset map: %s", _name.c_str());
	}
	_restartDue = 0;
	_pause = false;
	_mapRunning = false;
	_width = 0;
	_height = 0;
	_time = 0;
	_entityRemovalAllowed = true;
	if (!_name.empty())
		Log::info(LOG_MAP, "* clear map");

	{ // now free the allocated memory
		for (EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
			delete *i;
		}
		_entities.clear();
		_entities.reserve(400);

		for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
			delete *i;
		}
		_players.clear();
		_players.reserve(MAX_CLIENTS);
		if (!_name.empty())
			Log::info(LOG_MAP, "* removed allocated memory");
	}

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		delete *i;
	}
	_playersWaitingForSpawn.clear();
	_playersWaitingForSpawn.reserve(MAX_CLIENTS);

	if (!_name.empty())
		Log::info(LOG_MAP, "done with resetting: %s", _name.c_str());
	_name.clear();
}

inline IMapContext* getMapContext (const std::string& name)
{
	return new MiniRacerMapContext(name);
}

inline const EntityType& getEntityTypeForSpriteType (const SpriteType& spriteType)
{
	if (SpriteTypes::isDecal(spriteType))
		return EntityTypes::DECAL;
	if (SpriteTypes::isLand(spriteType))
		return EntityTypes::LAND;
	if (SpriteTypes::isRoad(spriteType))
		return EntityTypes::ROAD;
	if (SpriteTypes::isMoveable(spriteType))
		return EntityTypes::MOVEABLE;
	if (SpriteTypes::isVehicle(spriteType))
		return EntityTypes::PLAYER;
	return EntityTypes::SOLID;
}

bool Map::load (const std::string& name)
{
	std::unique_ptr<IMapContext> ctx(getMapContext(name));

	resetCurrentMap();

	if (name.empty()) {
		Log::info(LOG_MAP, "no map name given");
		return false;
	}

	Log::info(LOG_MAP, "load map %s", name.c_str());

	if (!ctx->load(false)) {
		Log::error(LOG_MAP, "failed to load the map %s", name.c_str());
		return false;
	}

	ctx->save();
	_settings = ctx->getSettings();
	_startPositions = ctx->getStartPositions();
	_name = ctx->getName();
	_title = ctx->getTitle();
	_width = getSetting(msn::WIDTH, "-1").toInt();
	_height = getSetting(msn::HEIGHT, "-1").toInt();

	if (_width <= 0 || _height <= 0) {
		Log::error(LOG_MAP, "invalid map dimensions given");
		return false;
	}

	const std::vector<MapTileDefinition>& mapTileList = ctx->getMapTileDefinitions();
	for (std::vector<MapTileDefinition>::const_iterator i = mapTileList.begin(); i != mapTileList.end(); ++i) {
		const SpriteType& t = i->spriteDef->type;
		Log::info(LOG_MAP, "sprite type: %s, %s", t.name.c_str(), i->spriteDef->id.c_str());
		MapTile *mapTile = new MapTile(*this, i->x, i->y, getEntityTypeForSpriteType(t));
		mapTile->setSpriteID(i->spriteDef->id);
		loadEntity(mapTile);
	}

	Log::info(LOG_MAP, "map loading done with %i tiles", (int)mapTileList.size());

	ctx->onMapLoaded();

	_frontend->onMapLoaded();
	const LoadMapMessage msg(_name, _title);
	_serviceProvider->getNetwork().sendToAllClients(msg);

	_mapRunning = true;
	return true;
}

bool Map::spawnPlayer (Player* player)
{
	assert(_entityRemovalAllowed);

	player->onSpawn();
	addEntity(0, *player);
	Log::info(LOG_SERVER, "spawned player %i", player->getID());
	_players.push_back(player);
	return true;
}

void Map::sendMessage (ClientId clientId, const std::string& message) const
{
	INetwork& network = _serviceProvider->getNetwork();
	network.sendToAllClients(TextMessage(message));
}

bool Map::isReadyToStart () const
{
	return _playersWaitingForSpawn.size() > 1;
}

void Map::startMap ()
{
	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		spawnPlayer(*i);
	}
	_playersWaitingForSpawn.clear();

	INetwork& network = _serviceProvider->getNetwork();
	network.sendToAllClients(StartMapMessage());
}

bool Map::initPlayer (Player* player)
{
	if (!_mapRunning)
		return false;

	if (getPlayer(player->getClientId()) != nullptr)
		return false;

	assert(_entityRemovalAllowed);

	INetwork& network = _serviceProvider->getNetwork();
	const ClientId clientId = player->getClientId();
	Log::info(LOG_SERVER, "init player %i", player->getID());
	const MapSettingsMessage mapSettingsMsg(_settings, _startPositions.size());
	network.sendToClient(clientId, mapSettingsMsg);

	const InitDoneMessage msgInit(player->getID(), 0, 0, 0);
	network.sendToClient(clientId, msgInit);

	network.sendToClient(clientId, InitWaitingMapMessage());
	sendMapToClient(clientId);
	if (!_players.empty()) {
		const bool spawned = spawnPlayer(player);
		return spawned;
	}
	_playersWaitingForSpawn.push_back(player);
	return true;
}

void Map::printPlayersList () const
{
	for (PlayerListConstIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		const std::string& name = (*i)->getName();
		Log::info(LOG_SERVER, "* %s (waiting)", name.c_str());
	}
	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
		const std::string& name = (*i)->getName();
		Log::info(LOG_SERVER, "* %s (spawned)", name.c_str());
	}
}

void Map::sendPlayersList () const
{
	std::vector<std::string> names;
	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
		const std::string& name = (*i)->getName();
		names.push_back(name);
	}
	for (PlayerListConstIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		const std::string& name = (*i)->getName();
		names.push_back(name);
	}
	INetwork& network = _serviceProvider->getNetwork();
	network.sendToAllClients(PlayerListMessage(names));
}

void Map::removeEntity (int clientMask, const IEntity& entity) const
{
	const RemoveEntityMessage msg(entity.getID(), false);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::addEntity (int clientMask, const IEntity& entity) const
{
	const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(entity.getAngle()));
	const AddEntityMessage msg(entity.getID(), entity.getType(), Animation::NONE,
			entity.getSpriteID(), entity.x(), entity.y(), entity.width(), entity.height(), angle, ENTITY_ALIGN_UPPER_LEFT);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::updateEntity (int clientMask, const IEntity& entity) const
{
	const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(entity.getAngle()));
	const UpdateEntityMessage msg(entity.getID(), 0, 0, angle, entity.getState());
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::sendMapToClient (ClientId clientId) const
{
	const int clientMask = ClientIdToClientMask(clientId);
	for (EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		const IEntity* e = *i;
		addEntity(clientMask, *e);
	}
}

void Map::loadEntity (IEntity *entity)
{
	assert(_entityRemovalAllowed);
	_entities.push_back(entity);
}

bool Map::removePlayer (ClientId clientId)
{
	assert(_entityRemovalAllowed);

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		if ((*i)->getClientId() != clientId)
			continue;
		(*i)->remove();
		delete *i;
		_playersWaitingForSpawn.erase(i);
		sendPlayersList();
		return true;
	}

	for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
		if ((*i)->getClientId() != clientId)
			continue;

		removeEntity(0, **i);
		(*i)->remove();
		delete *i;
		_players.erase(i);
		return true;
	}
	Log::error(LOG_MAP, "could not find the player with the clientId %i", clientId);
	return false;
}

bool Map::visitEntity (IEntity *entity)
{
	entity->update(Constant::DELTA_PHYSICS_MILLIS);
	return false;
}

void Map::autoStart () {
	// already spawned
	if (!_players.empty())
		return;
	// no players available yet
	if (_playersWaitingForSpawn.empty())
		return;
	// singleplayer already auto starts a map
	if (!_serviceProvider->getNetwork().isMultiplayer())
		return;
	// not enough players connected yet
	if (_playersWaitingForSpawn.size() < _startPositions.size())
		return;
	Log::info(LOG_SERVER, "starting the map");
	startMap();
}

void Map::update (uint32_t deltaTime)
{
	if (_pause)
		return;

	_timeManager.update(deltaTime);

	_time += deltaTime;

	ExecutionTime visitTime("VisitTime", 2000L);
	visitEntities(this);

	if (_restartDue > 0 && _restartDue <= _time) {
		const std::string currentName = getName();
		Log::info(LOG_MAP, "restarting map %s", currentName.c_str());
		load(currentName);
	}
}

const IEntity* Map::getEntity (int16_t id) const
{
	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i)
		if ((*i)->getID() == id)
			return *i;
	for (Map::EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i)
		if ((*i)->getID() == id)
			return *i;

	return nullptr;
}

void Map::visitEntities (IEntityVisitor *visitor, const EntityType& type)
{
	if (type == EntityType::NONE || type == EntityTypes::PLAYER) {
		bool needUpdate = false;
		for (PlayerListIter i = _players.begin(); i != _players.end();) {
			Player* e = *i;
			if (visitor->visitEntity(e)) {
				Log::debug(LOG_SERVER, "remove player by visit %i: %s", e->getID(), e->getType().name.c_str());
				removeEntity(ClientIdToClientMask(e->getClientId()), *e);
				delete *i;
				i = _players.erase(i);
				needUpdate = true;
			} else {
				++i;
			}
		}
		if (needUpdate) {
			if (_players.empty()) {
				resetCurrentMap();
				return;
			}
		}
	}

	// changing the entities list is not allowed here. Adding or removing
	// would invalidate the iterators
	for (Map::EntityListIter i = _entities.begin(); i != _entities.end();) {
		IEntity* e = *i;
		if (type.isNone() || e->getType() == type) {
			if (visitor->visitEntity(e)) {
				Log::debug(LOG_SERVER, "remove entity by visit %i: %s", e->getID(), e->getType().name.c_str());
				removeEntity(0, *e);
				(*i)->remove();
				delete *i;
				i = _entities.erase(i);
			} else {
				++i;
			}
		} else {
			++i;
		}
	}
}

void Map::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;
}

int Map::getMaxPlayers() const
{
	return _startPositions.size();
}

void Map::triggerPause ()
{
	if (!_serviceProvider->getNetwork().isServer())
		return;
	_pause ^= true;
	const PauseMessage msg(_pause);
	_serviceProvider->getNetwork().sendToAllClients(msg);
	Log::info(LOG_MAP, "pause: %s", (_pause ? "true" : "false"));
}

}
