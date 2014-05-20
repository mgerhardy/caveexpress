#include "Map.h"
#include "engine/server/box2d/DebugRenderer.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/MapSettings.h"
#include "engine/common/Shared.h"
#include "caveexpress/shared/constants/Commands.h"
#include "engine/common/Logger.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/common/IFrontend.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/IMapContext.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/PackageTarget.h"
#include "caveexpress/server/entities/Tree.h"
#include "caveexpress/server/entities/EntityEmitter.h"
#include "caveexpress/server/entities/WindowTile.h"
#include "caveexpress/server/map/LUAMapContext.h"
#include "caveexpress/server/map/RandomMapContext.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/server/entities/modificators/WindModificator.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/Geyser.h"
#include "caveexpress/server/entities/npcs/NPCAggressive.h"
#include "caveexpress/server/entities/npcs/NPCBlowing.h"
#include "caveexpress/server/entities/npcs/NPCFish.h"
#include "caveexpress/server/entities/npcs/NPCFlying.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/CaveExpressMapFailedReasons.h"
#include "caveexpress/shared/network/messages/UpdatePointsMessage.h"
#include "caveexpress/shared/network/messages/UpdatePackageCountMessage.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "engine/common/network/messages/InitDoneMessage.h"
#include "engine/common/network/messages/SoundMessage.h"
#include "engine/common/network/messages/MapSettingsMessage.h"
#include "engine/common/network/messages/TextMessage.h"
#include "engine/common/network/messages/SpawnInfoMessage.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/System.h"
#include "engine/common/vec2.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/Commands.h"
#include <SDL.h>
#include <algorithm>
#include <functional>
#include <cassert>

typedef SharedPtr<b2Shape> ShapePtr;
typedef std::multimap<std::string, ShapePtr> ShapeMap;
typedef ShapeMap::iterator ShapeIter;

#define SPAWN_FRIENDLY_NPC_DELAY 10000
#define SPAWN_FLYING_NPC_DELAY 5000
#define SPAWN_FISH_NPC_DELAY 5000

Map::Map () :
		IMap(), _world(nullptr), _frontend(nullptr), _serviceProvider(nullptr), _theme(&ThemeTypes::ROCK)
{
	Commands.registerCommand(CMD_MAP_PAUSE, bind(Map, triggerPause));
	Commands.registerCommand(CMD_MAP_RESTART, bind(Map, triggerRestart));
	Commands.registerCommand(CMD_MAP_DEBUG, bind(Map, triggerDebug));
	Commands.registerCommand(CMD_START, bind(Map, startMap));

	resetCurrentMap();
}

Map::~Map ()
{
	Commands.removeCommand(CMD_MAP_DEBUG);
	Commands.removeCommand(CMD_MAP_PAUSE);
	Commands.removeCommand(CMD_MAP_RESTART);
	clearPhysics();
}

void Map::shutdown ()
{
	resetCurrentMap();
}

void Map::addPoints (const IEntity* entity, uint16_t points)
{
	if (entity == nullptr)
		return;
	if (!entity->isPlayer())
		return;
	_gamePoints += points;
	_serviceProvider->getNetwork().sendToAllClients(UpdatePointsMessage(_gamePoints));
}

void Map::sendSound (int clientMask, const SoundType& type, const b2Vec2& pos) const
{
	const SoundMessage msg(pos.x, pos.y, type);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::sendSpawnInfo (const b2Vec2& pos, const EntityType& type) const
{
	const SpawnInfoMessage msg(pos.x, pos.y, type);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void Map::updateVisMask ()
{
	_allPlayers = 0;

	for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
		const Player* e = *i;
		const ClientId id = e->getClientId();
		_allPlayers |= ClientIdToClientMask(id);
	}

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		const Player* e = *i;
		const ClientId id = e->getClientId();
		_allPlayers |= ClientIdToClientMask(id);
	}
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

	info(LOG_MAP, "trigger restart");
	Commands.executeCommandLine(CMD_MAP_START " " + getName());
}

void Map::triggerDebug ()
{
	const bool newstate = Config.getDebugRenderer().activate ^ true;
	Config.setDebugRenderer(newstate, render, this);
	info(LOG_MAP, String::format("debug rendering: %s", newstate ? "true" : "false"));
}

void Map::triggerPause ()
{
	if (!_serviceProvider->getNetwork().isServer())
		return;
	_pause ^= true;
	GameEvent.notifyPause(_pause);
	info(LOG_MAP, String::format("pause: %s", _pause ? "true" : "false"));
}

void Map::render (void *userdata)
{
	Map* map = static_cast<Map*>(userdata);
	if (map->_world) {
		DebugRenderer renderer(map->_pointCount, map->_points, map->_traceCount, map->_traces,  map->_waterIntersectionPoints, Config.getMapDebugRect());
		map->_world->SetDebugDraw(&renderer);
		map->_world->DrawDebugData();
		map->_world->SetDebugDraw(nullptr);
	}
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

void Map::countTransferedPackage ()
{
	_transferedPackages++;
	info(LOG_SERVER, String::format("collected %i of %i packages", _transferedPackages, _transferedPackageLimit));
	const UpdatePackageCountMessage msg(getPackageCount());
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

int Map::getPackageCount () const
{
	return _transferedPackageLimit - _transferedPackages;
}

void Map::clearPhysics ()
{
	if (!_name.empty())
		info(LOG_MAP, "* clear physics");

	if (_world)
		_world->SetContactListener(nullptr);
	{ // delete the box2d stuff
		for (EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
			(*i)->prepareRemoval();
		}
		for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
			(*i)->prepareRemoval();
		}
		for (EntityListIter i = _entitiesToAdd.begin(); i != _entitiesToAdd.end(); ++i) {
			(*i)->prepareRemoval();
		}
		if (!_name.empty())
			info(LOG_MAP, "* removed box2d references");
	}

	{ // now free the allocated memory
		for (BorderList::iterator i = _borders.begin(); i != _borders.end(); ++i) {
			delete *i;
		}
		_borders.clear();

		for (EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
			delete *i;
		}
		for (EntityListIter i = _entitiesToAdd.begin(); i != _entitiesToAdd.end(); ++i) {
			delete *i;
		}
		_entitiesToAdd.clear();
		_entities.clear();
		_caves.clear();
		_platforms.clear();
		_entities.reserve(400);

		for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
			delete *i;
		}
		_players.clear();
		_players.reserve(MAX_CLIENTS);
		if (!_name.empty())
			info(LOG_MAP, "* removed allocated memory");
	}

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		delete *i;
	}
	_playersWaitingForSpawn.clear();
	_playersWaitingForSpawn.reserve(MAX_CLIENTS);

	if (_world)
		delete _world;
	if (!_name.empty())
		info(LOG_MAP, "* removed box2d world");
	_world = nullptr;
	_water = nullptr;
	_flyingNPC = nullptr;
	_fishNPC = nullptr;
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

	error(LOG_MAP, String::format("no player found for the client id %i", clientId));
	return nullptr;
}

bool Map::isFailed () const
{
	if (getWaterHeight() <= 0) {
		debug(LOG_MAP, "failed because water hit the top");
		return true;
	}

	if (_players.empty())
		return true;

	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
		const Player* player = *i;
		if (!player->isCrashed()) {
			return false;
		}
	}

	debug(LOG_MAP, String::format("failed because all %i players crashed", (int)_players.size()));
	return true;
}

const MapFailedReason& Map::getFailReason (const Player* player) const
{
	if (player->isCrashed()) {
		switch (player->getCrashReason()) {
		case CRASH_NPC_WALKING:
			return MapFailedReasons::FAILED_NPC_WALKING;
		case CRASH_NPC_MAMMUT:
			return MapFailedReasons::FAILED_NPC_MAMMUT;
		case CRASH_NPC_FISH:
			return MapFailedReasons::FAILED_NPC_FISH;
		case CRASH_NPC_FLYING:
			return MapFailedReasons::FAILED_NPC_FLYING;
		case CRASH_MAP_FAILED:
			return MapFailedReasons::FAILED_SIDESCROLL;
		default:
			return MapFailedReasons::FAILED_HITPOINTS;
		}
	}

	if (getWaterHeight() <= 0) {
		return MapFailedReasons::FAILED_WATER_HEIGHT;
	}

	return MapFailedReasons::FAILED_NO_MORE_PLAYERS;
}

int Map::handleDeadPlayers ()
{
	int deadPlayers = 0;
	PlayerList list = _players;
	for (PlayerListIter i = list.begin(); i != list.end(); ++i) {
		Player* p = *i;
		if (!p->isDead()) {
			continue;
		}

		const ClientId clientId = p->getClientId();
		info(LOG_MAP, "player " + p->getName() + " is dead");
		p->onDeath();
		disconnect(clientId);
		++deadPlayers;
	}
	return deadPlayers;
}

void Map::restart (uint32_t delay)
{
	if (_restartDue > 0)
		return;

	info(LOG_MAP, "trigger map restart");
	_restartDue = SDL_GetTicks() + delay;
	GameEvent.restartMap(delay);
}

void Map::resetCurrentMap ()
{
	_timeManager.reset();
	if (!_name.empty()) {
		GameEvent.closeMap();
		info(LOG_MAP, "reset map: " + _name);
	}
	_pointCount = 0;
	_traceCount = 0;
	_gamePoints = 0;
	_finishPoints = 0;
	_referenceTime = 0;
	_warmupPhase = 0;
	_restartDue = 0;
	_pause = false;
	_transferedPackages = 0;
	_transferedPackageLimit = 0;
	_nextFriendlyNPCSpawn = 0;
	_spawnFlyingNPCTime = 0;
	_activateflyingNPC = false;
	_spawnFishNPCTime = 0;
	_activateFishNPC = false;
	_mapRunning = false;
	_wind = 0.0f;
	_width = 0;
	_height = 0;
	_gravity = 0.0f;
	_time = 0;
	_physicsTime = 0;
	_waterHeight = 0.0f;
	_waterChangeSpeed = 0.0f;
	_waterRisingDelay = 0;
	_waterFallingDelay = 0;
	_allPlayers = 0;
	_entityRemovalAllowed = true;
	clearPhysics();
	if (!_name.empty())
		info(LOG_MAP, "done with resetting: " + _name);
	_name.clear();
}

inline IMapContext* getMapContext (const std::string& name)
{
	const std::string randomMapBase = "random";
	if (name.compare(0, randomMapBase.size(), randomMapBase) == 0) {
		const std::size_t themeSep = name.find("-");
		const ThemeType* theme = &ThemeTypes::ROCK;
		if (themeSep != std::string::npos) {
			const ThemeType& t = ThemeType::getByName(name.substr(themeSep + 1, name.size() - (themeSep + 1)));
			if (!t.isNone())
				theme = &t;
		}
		info(LOG_MAP, "use theme " + theme->name);
		RandomMapContext *ctx = new RandomMapContext(name, *theme, 8, 18, 20, 14);
		return ctx;
	}
	return new LUAMapContext(name);
}

bool Map::load (const std::string& name)
{
	ScopedPtr<IMapContext> ctx(getMapContext(name));

	resetCurrentMap();

	if (name.empty()) {
		info(LOG_MAP, "no map name given");
		return false;
	}

	info(LOG_MAP, "load map " + name);

	if (!ctx->load(false)) {
		error(LOG_MAP, "failed to load the map " + name);
		return false;
	}
	ctx->save();
	_settings = ctx->getSettings();
	_name = ctx->getName();
	_title = ctx->getTitle();
	_theme = &ctx->getTheme();
	_settings.insert(std::make_pair(msn::THEME, _theme->name));
	_wind = getSetting(msn::WIND, msd::WIND).toFloat();
	_gravity = getSetting(msn::GRAVITY, string::toString(msdv::GRAVITY)).toFloat();
	_width = getSetting(msn::WIDTH, "-1").toInt();
	_height = getSetting(msn::HEIGHT, "-1").toInt();
	_finishPoints = getSetting(msn::POINTS, string::toString(msdv::POINTS)).toInt();
	_referenceTime = getSetting(msn::REFERENCETIME, string::toString(msdv::REFERENCETIME)).toInt();
	_waterChangeSpeed = getSetting(msn::WATER_CHANGE, msd::WATER_CHANGE).toFloat();
	_waterRisingDelay = getSetting(msn::WATER_RISING_DELAY, msd::WATER_RISING_DELAY).toFloat();
	_waterFallingDelay = getSetting(msn::WATER_FALLING_DELAY, msd::WATER_FALLING_DELAY).toFloat();
	_activateflyingNPC = getSetting(msn::FLYING_NPC, msd::FLYING_NPC).toBool();
	_activateFishNPC = getSetting(msn::FISH_NPC, msd::FISH_NPC).toBool();
	_waterHeight = getSetting(msn::WATER_HEIGHT, msd::WATER_HEIGHT).toFloat();
	_transferedPackageLimit = getSetting(msn::PACKAGE_TRANSFER_COUNT, msd::PACKAGE_TRANSFER_COUNT).toInt();
	// TODO: properly implement a warmup phase
	_warmupPhase = 0;

	info(LOG_MAP, "theme: " + _theme->name);
	info(LOG_MAP, "reference time: " + string::toString(_referenceTime));

	if (_width <= 0 || _height <= 0) {
		error(LOG_MAP, "invalid map dimensions given");
		return false;
	}

	_spawnFlyingNPCTime = getSetting(msn::NPC_INITIAL_SPAWN_TIME, string::toString(4000 + rand() % SPAWN_FLYING_NPC_DELAY)).toInt();
	_spawnFishNPCTime = getSetting(msn::NPC_INITIAL_SPAWN_TIME, string::toString(4000 + rand() % SPAWN_FISH_NPC_DELAY)).toInt();

	if (_transferedPackageLimit <= 0) {
		error(LOG_MAP, "there is nothing to do in this map - set the npc or package limits");
		return false;
	}

	initPhysics();
	info(LOG_MAP, "physics initialized");

	std::vector<MapTile*> mapTilesWithBody;

	const std::vector<MapTileDefinition>& mapTileList = ctx->getMapTileDefinitions();
	for (std::vector<MapTileDefinition>::const_iterator i = mapTileList.begin(); i != mapTileList.end(); ++i) {
		MapTile *mapTile = createMapTileWithoutBody(i->spriteDef, i->x, i->y, i->angle);
		if (!mapTile->isDecoration() && !mapTile->isWindow())
			mapTilesWithBody.push_back(mapTile);
		loadEntity(mapTile);
	}

	const std::vector<CaveTileDefinition>& caveList = ctx->getCaveTileDefinitions();
	for (std::vector<CaveTileDefinition>::const_iterator i = caveList.begin(); i != caveList.end(); ++i) {
		MapTile *mapTile = new CaveMapTile(*this, i->spriteDef->id, i->x, i->y, *i->type, i->delay);
		mapTile->setGridDimensions(i->spriteDef->width, i->spriteDef->height, 0);
		mapTilesWithBody.push_back(mapTile);
		loadEntity(mapTile);
	}

	for (std::vector<MapTile*>::iterator i = mapTilesWithBody.begin(); i != mapTilesWithBody.end(); ++i) {
		MapTile* mapTile = *i;
		mapTile->createBody();
	}

	info(LOG_MAP, "init platforms");
	for (Map::EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
		IEntity* entity = *i;
		if (!entity->isGround())
			continue;
		MapTile *mapTile = static_cast<MapTile*>(entity);
		int start = -1;
		int end = -1;
		const int y = mapTile->getGridY() - 1.0f + EPSILON;
		getPlatformDimensions(mapTile->getGridX(), y, &start, &end);
		if (start == -1 || end == -1) {
			continue;
		}
		getPlatform(mapTile, &start, &end);
	}

	info(LOG_MAP, "init caves");
	for (Map::EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
		IEntity* entity = *i;
		if (!entity->isCave())
			continue;
		CaveMapTile *cave = static_cast<CaveMapTile*>(entity);
		_caves.push_back(cave);
	}

	// do another loop when we have all caves - we have to know each of the caves in order to initialize them properly
	for (Map::CaveListIter i = _caves.begin(); i != _caves.end(); ++i) {
		CaveMapTile* cave = *i;
		initCave(cave);
	}

	const std::vector<EmitterDefinition>& emitterList = ctx->getEmitterDefinitions();
	for (std::vector<EmitterDefinition>::const_iterator i = emitterList.begin(); i != emitterList.end(); ++i) {
		const EntityType &type = *i->type;
		if (type.isNone())
			continue;
		EntityEmitter *entity = new EntityEmitter(*this, i->x, i->y, i->amount, i->delay, type, i->settings);
		loadEntity(entity);
	}

	if (!hasPackageTarget()) {
		error(LOG_MAP, "there is no package target in this map");
		return false;
	}

	info(LOG_MAP, "map loading done");

	ctx->onMapLoaded();

	_frontend->onMapLoaded();
	GameEvent.loadMap(0, _name, _title);

	_mapRunning = true;
	return true;
}

class TraceCallback: public b2RayCastCallback {
private:
	float _fraction;
	IEntity *_entity;

public:
	TraceCallback () :
			_fraction(0.0), _entity(nullptr)
	{
	}

	float32 ReportFixture (b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
	{
		IEntity *e = static_cast<IEntity *>(fixture->GetBody()->GetUserData());
		if (e && (e->isSolid() || e->isBorder())) {
			_fraction = fraction;
			_entity = e;
			return fraction;
		}
		return -1;
	}

	// the length of the way the trace came along until it hit some obstacle - 1.0 if nothing was hit
	inline float getFraction () const
	{
		return _fraction;
	}

	// the entity that was hit
	inline IEntity* getEntity () const
	{
		return _entity;
	}
};

bool Map::isReachableByWalking (const IEntity *start, const IEntity *end, int startPos, int endPos) const
{
	// check that there is nothing solid in between
	IEntity* entity = nullptr;
	trace(start, end, &entity);
	if (entity != nullptr && entity->isSolid())
		return false;

	if (startPos == -1 || endPos == -1)
		getPlatformDimensions(static_cast<int>(start->getPos().x), static_cast<int>(start->getPos().y), &startPos, &endPos);

	// if there is a start and end pos of a platform given, then let's check whether end is inside the range
	const gridCoord xStart = start->getPos().x;
	const gridCoord xEnd = end->getPos().x;
	if (xStart < xEnd) {
		// walking right
		return xEnd <= endPos;
	}
	// walking left
	return startPos <= xEnd;
}

bool Map::trace (const b2Vec2& start, const b2Vec2& end, IEntity **hit) const
{
	TraceCallback callback;
	_world->RayCast(&callback, start, end);
	if (hit)
		*hit = callback.getEntity();

	int index = _traceCount;
	if (index < SDL_arraysize(_traces)) {
		_traces[index].start = start;
		_traces[index].end = end;
		_traces[index].fraction = callback.getFraction();
		_traceCount++;
	}

	return callback.getFraction() < 1.0f;
}

bool Map::trace (const IEntity *start, const IEntity *end, IEntity **hit) const
{
	return trace(start->getPos(), end->getPos(), hit);
}

bool Map::trace (int startGridX, int startGridY, int endGridX, int endGridY, IEntity **hit) const
{
	// center of the cells
	const b2Vec2 start(startGridX + 0.5f, startGridY + 0.5f);
	const b2Vec2 end(endGridX + 0.5f, endGridY + 0.5f);
	return trace(start, end, hit);
}

bool Map::spawnPlayer (Player* player)
{
	assert(_entityRemovalAllowed);

	info(LOG_SERVER, "spawn player " + player->toString());
	const float playerStartX = getSetting(msn::PLAYER_X, msd::PLAYER_X).toFloat();
	const float playerStartY = getSetting(msn::PLAYER_Y, msd::PLAYER_Y).toFloat();
	const b2Vec2& size = player->getSize();
	const b2Vec2 pos(playerStartX + size.x / 2.0f, playerStartY + size.y / 2.0f);
	player->createBody(pos);
	player->onSpawn();
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
	updateVisMask();

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
	info(LOG_SERVER, "init player " + player->toString());
	const int clientMask = ClientIdToClientMask(clientId);
	const MapSettingsMessage mapSettingsMsg(_settings);
	network.sendToClient(clientId, mapSettingsMsg);
	GameEvent.sendWaterUpdate(clientMask, *_water);

	const InitDoneMessage msgInit(player->getID(), getPackageCount(), player->getLives(), player->getHitpoints());
	network.sendToClient(clientId, msgInit);

	sendSound(0, SoundTypes::SOUND_PLAYER_SPAWN);

	network.sendToClient(clientId, InitWaitingMapMessage());
	updateVisMask();
	sendMapToClient(clientId);
	if (!_players.empty()) {
		const bool spawned = spawnPlayer(player);
		updateVisMask();
		return spawned;
	}
	_playersWaitingForSpawn.push_back(player);
	return true;
}

void Map::printPlayersList () const
{
	std::vector<std::string> names;
	for (PlayerListConstIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		const std::string& name = (*i)->getName();
		info(LOG_SERVER, "* " + name + " (waiting)");
	}
	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
		const std::string& name = (*i)->getName();
		info(LOG_SERVER, "* " + name + " (spawned)");
	}
}

void Map::sendPlayersList () const
{
	std::vector<std::string> names;
	for (PlayerListConstIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		const std::string& name = (*i)->getName();
		names.push_back(name);
	}
	INetwork& network = _serviceProvider->getNetwork();
	network.sendToAllClients(PlayerListMessage(names));
}

void Map::initPhysics ()
{
	b2Vec2 gravity;
	gravity.Set(0.0f, getGravity());
	_world = new b2World(gravity);

	_world->SetDestructionListener(&_destructionListener);

	//_world->SetWarmStarting(false);
	//_world->SetContinuousPhysics(false);
	//_world->SetSubStepping(false);
	_world->SetAutoClearForces(true);
	_world->SetContactListener(this);
	_world->SetContactFilter(this);

	const float zeroX = 0.0f;
	const float zeroY = -0.5f;
	const float width = getMapWidth();
	// added a small offset to allow water diving out of screen
	const float height = getMapHeight() + 1.0f;

	b2BodyDef lineBodyDef;
	lineBodyDef.type = b2_staticBody;
	lineBodyDef.position.Set(0, 0);
	b2Body* boxBody = _world->CreateBody(&lineBodyDef);

	b2EdgeShape edge;
	b2FixtureDef fd;
	fd.friction = 1.0f;
	fd.restitution = 0.2f;
	fd.shape = &edge;

	_borders.resize(BORDER_MAX);
	const bool isSideBorderFail = getSetting(msn::SIDEBORDERFAIL).toBool();
	_borders[BORDER_TOP] = new Border(BorderType::TOP, *this);
	_borders[BORDER_LEFT] = new Border(BorderType::LEFT, *this, isSideBorderFail);
	_borders[BORDER_RIGHT] = new Border(BorderType::RIGHT, *this, isSideBorderFail);
	_borders[BORDER_BOTTOM] = new Border(BorderType::BOTTOM, *this);
	_borders[BORDER_PLAYER_BOTTOM] = new Border(BorderType::PLAYER_BOTTOM, *this);

	edge.Set(b2Vec2(zeroX, zeroY), b2Vec2(width, zeroY));
	b2Fixture *top = boxBody->CreateFixture(&fd);
	top->SetUserData(_borders[BORDER_TOP]);

	edge.Set(b2Vec2(zeroX, zeroY), b2Vec2(zeroX, height));
	b2Fixture *left = boxBody->CreateFixture(&fd);
	left->SetUserData(_borders[BORDER_LEFT]);

	edge.Set(b2Vec2(width, height), b2Vec2(width, zeroY));
	b2Fixture *right = boxBody->CreateFixture(&fd);
	right->SetUserData(_borders[BORDER_RIGHT]);

	edge.Set(b2Vec2(zeroX, height), b2Vec2(width, height));
	b2Fixture *bottom = boxBody->CreateFixture(&fd);
	bottom->SetUserData(_borders[BORDER_BOTTOM]);

	edge.Set(b2Vec2(zeroX, height), b2Vec2(width, getMapHeight()));
	b2Fixture *playerBottom = boxBody->CreateFixture(&fd);
	playerBottom->SetUserData(_borders[BORDER_PLAYER_BOTTOM]);

	initWater();
}

void Map::initCave (CaveMapTile* caveTile)
{
	int start = -1;
	int end = -1;
	Platform *platform = getPlatform(caveTile, &start, &end, caveTile->getSize().y);
	if (platform == nullptr) {
		error(LOG_MAP, "failed to initialize the cave platform");
		return;
	}
	platform->setCave(caveTile);
	initWindows(caveTile, start, end);
	caveTile->setRespawnPossible(true, EntityType::NONE);
	caveTile->setPlatformDimensions(start, end);
}

void Map::initWindows (CaveMapTile* caveTile, int start, int end)
{
	const int x = caveTile->getGridX();
	const int left = std::max(x - 2, start);
	const int right = std::max(x + 2, end);
	const int caveY = caveTile->getGridY();
	for (EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		IEntity* e = *i;
		if (!e->isWindow())
			continue;
		WindowTile* window = static_cast<WindowTile*>(e);
		if (window->getGridY() != caveY)
			continue;
		for (int gridX = left; gridX < right; ++gridX) {
			const int windowX = window->getGridX();
			if (Between(windowX, left, right)) {
				caveTile->addWindow(window);
				break;
			}
		}
	}
}

Platform *Map::getPlatform (MapTile *mapTile, int *start, int *end, gridSize offset)
{
	const int mapY = mapTile->getGridY() + offset + EPSILON;
	if (*start == -1 || *end == -1)
		getPlatformDimensions(mapTile->getGridX(), mapTile->getGridY(), start, end);

	PlatformYMapConstIter iy = _platforms.find(mapY);
	if (iy != _platforms.end()) {
		PlatformXMapConstIter ix = iy->second.find(*start);
		if (ix != iy->second.end()) {
			return ix->second;
		}
	}

	info(LOG_MAP, String::format("create a new platform at %i:%i to %i:%i", *start, mapY, *end, mapY));
	const int width = *end - *start + 1;
	const gridSize height = 0.015f;
	const gridCoord x = *start + width / 2.0f;
	const gridSize y = mapTile->getGridY() + offset;

	b2PolygonShape shape;
	shape.SetAsBox(width / 2.0f, height);

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.friction = 0.4f;
	fixture.restitution = 0.0f;
	fixture.density = 0.0f;

	b2BodyDef bd;
	bd.position.Set(x, y);
	bd.type = b2_kinematicBody;
	bd.fixedRotation = true;

	Platform *platform = new Platform(*this);
	addToWorld(fixture, bd, platform);
	loadEntity(platform);

	_platforms[mapY][*start] = platform;

#ifdef DEBUG
	PlatformYMapConstIter iy2 = _platforms.find(mapY);
	assert(iy2 != _platforms.end());
	PlatformXMapConstIter ix2 = iy2->second.find(*start);
	assert(ix2 != iy->second.end());
#endif

	return platform;
}

void Map::getPlatformDimensions (int gridX, int startTraceGridY, int *start, int *end) const
{
	const int endTraceGridY = startTraceGridY + 1;

	if (gridX > 0) {
		IEntity *hit = nullptr;
		trace(gridX, startTraceGridY, 0, startTraceGridY, &hit);
		int leftGridX = 0;
		if (hit && hit->isSolid()) {
			MapTile *mapTile = static_cast<MapTile*>(hit);
			leftGridX = mapTile->getGridX() + mapTile->getGridWidth();
		}
		int startTraceGridX = gridX;
		const int steps = startTraceGridX - leftGridX;
		for (int i = 0; i < steps; ++i) {
			const b2Vec2 startV(startTraceGridX - 0.5f, startTraceGridY + 0.5f);
			const b2Vec2 endV(startTraceGridX - 0.5f, endTraceGridY + 0.2f);
			const bool state = trace(startV, endV, &hit);
			if (state && hit && hit->isSolid()) {
				--startTraceGridX;
				continue;
			}
			break;
		}
		*start = startTraceGridX;
	} else {
		IEntity *hit = nullptr;
		const b2Vec2 startV(0, startTraceGridY);
		const b2Vec2 endV(0, startTraceGridY + 0.0001f);
		const bool state = trace(startV, endV, &hit);
		if (state && hit && hit->isSolid())
			return;

		*start = 0;
	}

	if (gridX < _width - 1) {
		IEntity *hit = nullptr;
		trace(gridX, startTraceGridY, _width - 1, startTraceGridY, &hit);
		int rightGridX = _width - 1;
		if (hit && hit->isSolid()) {
			MapTile *mapTile = static_cast<MapTile*>(hit);
			// we always subtract 1 here - not the width - it's the left side
			rightGridX = mapTile->getGridX() - 1;
		}
		int endTraceGridX = gridX;
		const int steps = rightGridX - endTraceGridX;
		for (int i = 0; i < steps; ++i) {
			const b2Vec2 startV(endTraceGridX + 1.5f, startTraceGridY + 0.5f);
			const b2Vec2 endV(endTraceGridX + 1.5f, endTraceGridY + 0.2f);
			const bool state = trace(startV, endV, &hit);
			if (state && hit && hit->isSolid()) {
				++endTraceGridX;
				continue;
			}
			break;
		}
		*end = endTraceGridX;
	} else {
		IEntity *hit = nullptr;
		const b2Vec2 startV(_width - 1.0f, startTraceGridY);
		const b2Vec2 endV(_width - 1.0f, startTraceGridY + 0.0001f);
		const bool state = trace(startV, endV, &hit);
		if (state && hit && hit->isSolid())
			return;

		*end = _width - 1;
	}
}

void Map::initWater ()
{
	_water = new Water(*this, _waterChangeSpeed, _waterRisingDelay, _waterFallingDelay);
	_water->createBody(_waterHeight);
}

MapTile* Map::createMapTileWithoutBody (const SpriteDefPtr& spriteDef, gridCoord gridX, gridCoord gridY, EntityAngle angle)
{
	MapTile* mapTile;
	const SpriteType& type = spriteDef->type;
	if (SpriteTypes::isCave(type)) {
		mapTile = new CaveMapTile(*this, spriteDef->id, gridX, gridY);
	} else if (SpriteTypes::isBackground(type)) {
		mapTile = new MapTile(*this, spriteDef->id, gridX, gridY, EntityTypes::DECORATION);
	} else if (SpriteTypes::isWindow(type)) {
		mapTile = new WindowTile(*this, spriteDef->id, gridX, gridY);
	} else if (SpriteTypes::isLava(type)) {
		mapTile = new MapTile(*this, spriteDef->id, gridX, gridY, EntityTypes::LAVA);
	} else if (SpriteTypes::isPackageTarget(type)) {
		mapTile = new PackageTarget(*this, spriteDef->id, gridX, gridY);
	} else if (SpriteTypes::isGeyser(type)) {
		mapTile = new Geyser(*this, spriteDef->id, gridX, gridY);
	} else if (SpriteTypes::isAnyGround(type) || SpriteTypes::isBridge(type)) {
		mapTile = new MapTile(*this, spriteDef->id, gridX, gridY, EntityTypes::GROUND);
	} else if (SpriteTypes::isSolid(type)) {
		mapTile = new MapTile(*this, spriteDef->id, gridX, gridY, EntityTypes::SOLID);
	} else {
		mapTile = new MapTile(*this, spriteDef->id, gridX, gridY, EntityTypes::DECORATION);
	}
	mapTile->setGridDimensions(spriteDef->width, spriteDef->height, angle);
	return mapTile;
}

b2Body* Map::addToWorld (b2FixtureDef &fixtureDef, b2BodyDef &bodyDef, IEntity *entity)
{
	assert(_entityRemovalAllowed);

	SpriteDefPtr def = entity->getSpriteDef();
	if (def) {
		if (def->hasShape())
			fixtureDef.shape = nullptr;
		fixtureDef.restitution = def->restitution;
		fixtureDef.friction = def->friction;
	}

	b2Body* body = _world->CreateBody(&bodyDef);
	body->SetUserData(entity);

	if (fixtureDef.shape != nullptr) {
		b2Fixture* fixture = body->CreateFixture(&fixtureDef);
		fixture->SetUserData(const_cast<char*>(""));
		entity->addBody(body);
		return body;
	}

	if (!def) {
		error(LOG_MAP, "no shape given - could not find sprite definition for " + entity->getType().name);
		return nullptr;
	}

	// create the shape from the sprite definition polygons
	const int polygons = def->polygons.size();
	for (int j = 0; j < polygons; ++j) {
		const SpritePolygon& polygon = def->polygons[j];
		const int cnt = polygon.vertices.size();
		int vertexCnt = 0;
		b2Vec2 points[b2_maxPolygonVertices];
		const int size = SDL_arraysize(points);
		if (cnt > size)
			error(LOG_MAP, "too many vertices given for sprite " + def->id);

		for (int i = 0; i < cnt; ++i) {
			const SpriteVertex &v = polygon.vertices[i];
			points[vertexCnt].x = v.x;
			points[vertexCnt].y = v.y * -1.0f;
			vertexCnt++;
			if (vertexCnt >= size)
				break;
		}

		if (vertexCnt == 0)
			continue;

		b2PolygonShape shape;
		shape.Set(points, vertexCnt);
		fixtureDef.shape = &shape;
		b2Fixture* fixture = body->CreateFixture(&fixtureDef);
		fixture->SetUserData(const_cast<char*>(polygon.userData.c_str()));
	}

	// create the shape from the sprite definition circles
	for (std::vector<SpriteCircle>::const_iterator i = def->circles.begin(); i != def->circles.end(); ++i) {
		const SpriteCircle& circle = *i;
		b2CircleShape shape;
		shape.m_p = b2Vec2(circle.center.x, circle.center.y);
		shape.m_radius = circle.radius;
		fixtureDef.shape = &shape;
		b2Fixture* fixture = body->CreateFixture(&fixtureDef);
		fixture->SetUserData(const_cast<char*>(circle.userData.c_str()));
	}

	entity->addBody(body);
	return body;
}

void Map::addEntity (IEntity *entity)
{
	entity->onSpawn();
	_entitiesToAdd.push_back(entity);
	const VisMask vismask = entity->getVisMask();
	handleVisibility(entity, vismask);
}

void Map::sendMapToClient (ClientId clientId) const
{
	const VisMask clientMask = ClientIdToClientMask(clientId);

	for (EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		if (!(*i)->isMapTile())
			continue;
		calculateVisibility(*i);
		sendVisibleEntity(clientMask, *i);
	}
}

void Map::loadEntity (IEntity *entity)
{
	assert(_entityRemovalAllowed);
	//entity->onSpawn();
	_entities.push_back(entity);
}

PackageTarget *Map::getPackageTarget () const
{
	std::vector<const PackageTarget*> packageTargets;
	for (EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		if (!(*i)->isPackageTarget())
			continue;
		const PackageTarget *packageTarget = static_cast<const PackageTarget*>(*i);
		packageTargets.push_back(packageTarget);
	}
	const int packageTargetCount = packageTargets.size();
	if (packageTargetCount == 0) {
		info(LOG_MAP, "no package target found");
		return nullptr;
	}

	if (packageTargetCount == 1)
		return const_cast<PackageTarget*>(packageTargets[0]);

	const int randomPackageTarget = rand() % packageTargetCount;
	return const_cast<PackageTarget*>(packageTargets[randomPackageTarget]);
}

CaveMapTile *Map::getHighestCave () const
{
	// get the highest cave to rescue the npcs from the rising water
	float y = getMapHeight();
	CaveMapTile *cave = nullptr;
	for (CaveListConstIter i = _caves.begin(); i != _caves.end(); ++i) {
		CaveMapTile *caveTile = *i;
		const float caveY = caveTile->getPos().y;
		if (caveY < y) {
			y = caveY;
			cave = caveTile;
		}
	}
	return cave;
}

CaveMapTile *Map::getTargetCave (const CaveMapTile* ignoreCave) const
{
	if (isWaterRising()) {
		return getHighestCave();
	}
	CaveList tmp;
	for (CaveList::const_iterator i = _caves.begin(); i != _caves.end(); ++i) {
		CaveMapTile *cave = *i;
		if (cave->isUnderWater() || ignoreCave == cave)
			continue;
		tmp.push_back(cave);
	}

	if (tmp.empty()) {
		debug(LOG_MAP, "no usable cave found");
		return nullptr;
	}

	const int randomCave = rand() % tmp.size();
	return tmp[randomCave];
}

bool Map::removePlayer (ClientId clientId)
{
	assert(_entityRemovalAllowed);

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		if ((*i)->getClientId() != clientId)
			continue;
		(*i)->prepareRemoval();
		delete *i;
		_playersWaitingForSpawn.erase(i);
		sendPlayersList();
		updateVisMask();
		return true;
	}

	for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
		if ((*i)->getClientId() != clientId)
			continue;

		for (EntityListIter refIter = _entities.begin(); refIter != _entities.end(); ++refIter) {
			if (!(*refIter)->isNpcAttacking())
				continue;
			NPCAttacking *npcAttacking = static_cast<NPCAttacking*>(*refIter);
			npcAttacking->stopAttack(*i);
		}

		GameEvent.removeEntity((*i)->getVisMask(), **i);
		(*i)->prepareRemoval();
		delete *i;
		_players.erase(i);
		updateVisMask();
		return true;
	}
	error(LOG_MAP, String::format("could not find the player with the clientId %i", clientId));
	return false;
}

NPCBlowing* Map::createBlowingNPC (const b2Vec2& pos, bool right, float force, float modificatorSize)
{
	assert(_entityRemovalAllowed);

	NPCBlowing *npc = new NPCBlowing(*this, pos, right, force, modificatorSize);
	addEntity(npc);

	return npc;
}

NPCAttacking* Map::createAttackingNPC (const b2Vec2& pos, const EntityType& entityType, bool right)
{
	assert(EntityTypes::isNpcAttacking(entityType));
	assert(_entityRemovalAllowed);
	NPCAttacking *npc = new NPCAttacking(entityType, *this, right);
	npc->createBody(pos, _world);
	npc->calculatePlatformDimensions();
	addEntity(npc);
	return npc;
}

NPCFish* Map::createFishNPC (const b2Vec2& pos)
{
	assert(_entityRemovalAllowed);
	NPCFish *npc = new NPCFish(*this);
	npc->createBody(pos, false, true);
	addEntity(npc);
	return npc;
}

NPCFlying* Map::createFlyingNPC (const b2Vec2& pos)
{
	assert(_entityRemovalAllowed);
	NPCFlying *npc = new NPCFlying(*this);
	npc->createBody(pos);
	addEntity(npc);
	return npc;
}

NPCPackage* Map::createPackageNPC (CaveMapTile* cave, const EntityType& type)
{
	assert(_entityRemovalAllowed);

	if (getPackageTarget() == nullptr)
		return nullptr;

	NPCPackage* npc = new NPCPackage(cave, type);
	addEntity(npc);

	visitEntity(npc);

	return npc;
}

inline void Map::calculateVisibility (IEntity *entity) const
{
	// static objects are always visible - there is no need to make them
	// invisible, as they are not updated anyway
	if (entity->isMapTile()) {
		entity->setVisMask(_allPlayers);
	} else if (entity->isDynamic() || entity->isTree()) {
		VisMask visMask = 0;
		for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
			const Player* e = *i;
			if (entity->isVisibleFor(e)) {
				const ClientId id = e->getClientId();
				visMask |= ClientIdToClientMask(id);
			}
		}
		if (visMask == 0)
			visMask = NOTVISIBLE;
		entity->setVisMask(visMask);
	}
}

void Map::handleVisibility (IEntity *entity, const VisMask vismask) const
{
	calculateVisibility(entity);
	const VisMask newVismask = entity->getVisMask();
	const VisMask delta = vismask ^ newVismask;
	int removeMask = 0;
	int addMask = 0;
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		const int clientMask = ClientIdToClientMask(i);
		if (delta & clientMask) {
			if (newVismask & clientMask) {
				addMask |= clientMask;
			} else {
				removeMask |= clientMask;
			}
		}
	}

	if (removeMask != 0) {
		//info(LOG_MAP, String::format("server: remove entity %i type: %s", entity->getID(), entity->getType().name.c_str()));
		GameEvent.removeEntity(removeMask, *entity);
	}

	if (addMask != 0) {
		sendVisibleEntity(addMask, entity);
	}
}

void Map::sendVisibleEntity (int clientMask, const IEntity *entity) const
{
	//debug(LOG_MAP, String::format("server: add entity %i type: %s", entity->getID(), entity->getType().name.c_str()));
	GameEvent.addEntity(clientMask, *entity);
	if (entity->isCave()) {
		const CaveMapTile *tile = static_cast<const CaveMapTile *>(entity);
		GameEvent.addCave(clientMask, entity->getID(), tile->getLightState());
	} else if (entity->isWindow()) {
		const WindowTile *tile = static_cast<const WindowTile *>(entity);
		GameEvent.sendLightState(clientMask, tile->getID(), tile->getLightState());
	}
}

bool Map::visitEntity (IEntity *entity)
{
	const VisMask vismask = entity->getVisMask();
	if (_time >= _warmupPhase) {
		entity->update(Constant::DELTA_PHYSICS_MILLIS);
		if (entity->shouldApplyWind())
			entity->applyLinearImpulse(b2Vec2(_wind, 0.0f));
	}
	handleVisibility(entity, vismask);

	return entity->isRemove();
}

void Map::handleFlyingNPC ()
{
	if (!_activateflyingNPC)
		return;

	if (_spawnFlyingNPCTime > _time)
		return;

	const float gap = 2.0f;
	if (_flyingNPC == nullptr) {
		if (_players.empty())
			return;

		const int index = rand() % _players.size();
		const Player* player = _players[index];
		const b2Vec2& pos = player->getPos();
		const float waterBodyY = getWaterHeight();
		float y = pos.y;
		if (y >= waterBodyY) {
			y = waterBodyY - 1.0f;
		}
		if (y < 0.f) {
			return;
		}

		float x;
		if (pos.x > getMapWidth() / 2.0)
			x = -gap;
		else
			x = getMapWidth() + gap;
		const b2Vec2 npcSpawnPos(x, y);
		_flyingNPC = createFlyingNPC(npcSpawnPos);
	} else {
		const b2Vec2 &pos = _flyingNPC->getPos();
		const float x = pos.x;
		const float y = pos.y;
		if (x < -gap || y < 0 || x > getMapWidth() + gap || y > getMapHeight()) {
			_flyingNPC->setRemove();
			_spawnFlyingNPCTime = _time + 2000 + rand() % SPAWN_FLYING_NPC_DELAY;
			_flyingNPC = nullptr;
		}
	}
}

void Map::handleFishNPC ()
{
	if (!_activateFishNPC)
		return;

	const float waterBodyY = getWaterHeight();
	// no fish if water is too low
	if (waterBodyY <= 0.5f)
		return;

	if (_spawnFishNPCTime > _time)
		return;

	const float gap = 2.0f;
	if (_fishNPC == nullptr) {
		if (_players.empty())
			return;

		const int index = rand() % _players.size();
		const Player* player = _players[index];
		const b2Vec2& pos = player->getPos();
		const float mapHeight = static_cast<float>(getMapHeight());
		float y = std::min(waterBodyY, std::max(mapHeight, mapHeight - 0.5f));
		if (y < 0.f) {
			return;
		}

		float x;
		if (pos.x > getMapWidth() / 2.0)
			x = -gap;
		else
			x = getMapWidth() + gap;
		const b2Vec2 npcSpawnPos(x, y);
		_fishNPC = createFishNPC(npcSpawnPos);
	} else {
		const b2Vec2 &pos = _fishNPC->getPos();
		const float x = pos.x;
		const float y = pos.y;
		if (x < -gap || y < 0 || x > getMapWidth() + gap || y > getMapHeight()) {
			_fishNPC->setRemove();
			_spawnFishNPCTime = _time + 2000 + rand() % SPAWN_FISH_NPC_DELAY;
			_fishNPC = nullptr;
		}
	}
}

void Map::update (uint32_t deltaTime)
{
	_pointCount = 0;

	if (_pause)
		return;

	_timeManager.update(deltaTime);

	if (_restartDue > 0 && _restartDue <= SDL_GetTicks()) {
		const std::string currentName = _name;
		info(LOG_MAP, "restarting map " + currentName);
		if (isFailed()) {
			const Map::PlayerList& players = getPlayers();
			for (Map::PlayerListConstIter i = players.begin(); i != players.end(); ++i) {
				const Player* p = *i;
				System.track("MapState", "failed:" + getName());
				GameEvent.failedMap(p->getClientId(), getName(), getFailReason(p), getTheme());
			}
		} else {
			load(currentName);
		}
		return;
	}

	if (_world) {
		_time += deltaTime;
		while (_time - _physicsTime >= Constant::DELTA_PHYSICS_MILLIS) {
			_physicsTime += Constant::DELTA_PHYSICS_MILLIS;
			{
				ExecutionTime visitTime("VisitTime", 2000L);
				visitEntities(this);
			}

			handleFlyingNPC();
			handleFishNPC();

			if (_time >= _warmupPhase) {
				_entityRemovalAllowed = false;
				ExecutionTime stepTime("StepTime", 2000L);
				_world->Step(Constant::DELTA_PHYSICS_SECONDS, 8, 3);
				_entityRemovalAllowed = true;
			}
		}

		const int t = _referenceTime - _time / 1000;
		if (t < 0)
			return;

		static int lastT = 0;
		if (lastT != t) {
			GameEvent.sendTimeRemaining(t);
			lastT = t;
		}
	}
}

bool Map::ShouldCollide (b2Fixture* fixtureA, b2Fixture* fixtureB)
{
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA->GetBody()->GetUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB->GetBody()->GetUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA->GetUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB->GetUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		const bool shouldCollide = entity1->shouldCollide(entity2) || entity2->shouldCollide(entity1);
		if (entity1->shouldRefilter())
			fixtureA->Refilter();
		if (entity2->shouldRefilter())
			fixtureB->Refilter();
		return shouldCollide;
	}

	const b2Filter& filterA = fixtureA->GetFilterData();
	const b2Filter& filterB = fixtureB->GetFilterData();
	if (filterA.groupIndex == filterB.groupIndex && filterA.groupIndex != 0) {
		return filterA.groupIndex > 0;
	}

	const bool collide = (filterA.maskBits & filterB.categoryBits) != 0 && (filterA.categoryBits & filterB.maskBits) != 0;
	return collide;
}

void Map::BeginContact (b2Contact* contact)
{
	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA->GetBody()->GetUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB->GetBody()->GetUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA->GetUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB->GetUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		entity1->onContact(contact, entity2);
		entity2->onContact(contact, entity1);
	}
}

void Map::EndContact (b2Contact* contact)
{
	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA->GetBody()->GetUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB->GetBody()->GetUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA->GetUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB->GetUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		entity1->endContact(contact, entity2);
		entity2->endContact(contact, entity1);
	}
}

void Map::PostSolve (b2Contact* contact, const b2ContactImpulse* impulse)
{
	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA->GetBody()->GetUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB->GetBody()->GetUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA->GetUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB->GetUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		entity1->onPostSolve(contact, impulse, entity2);
		entity2->onPostSolve(contact, impulse, entity1);
	}
}

void Map::PreSolve (b2Contact* contact, const b2Manifold* oldManifold)
{
	b2Fixture *fixtureA = contact->GetFixtureA();
	b2Fixture *fixtureB = contact->GetFixtureB();
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA->GetBody()->GetUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB->GetBody()->GetUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA->GetUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB->GetUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		entity1->onPreSolve(contact, entity2, oldManifold);
		entity2->onPreSolve(contact, entity1, oldManifold);
	}

	const b2Manifold* manifold = contact->GetManifold();

	if (manifold->pointCount == 0) {
		return;
	}

	b2PointState state1[b2_maxManifoldPoints], state2[b2_maxManifoldPoints];
	b2GetPointStates(state1, state2, oldManifold, manifold);

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);

	for (int32_t i = 0; i < manifold->pointCount && _pointCount < MAXCONTACTPOINTS; ++i) {
		ContactPoint* cp = _points + _pointCount;
		cp->fixtureA = fixtureA;
		cp->fixtureB = fixtureB;
		cp->position = worldManifold.points[i];
		cp->normal = worldManifold.normal;
		cp->state = state2[i];
		cp->normalImpulse = manifold->points[i].normalImpulse;
		cp->tangentImpulse = manifold->points[i].tangentImpulse;
		++_pointCount;
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

int Map::countPackages () const
{
	int packages = 0;
	for (Map::EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		if (!(*i)->isPackage())
			continue;
		const Package *package = static_cast<Package*>(*i);
		if (package->isCounted())
			continue;
		++packages;
	}
	return packages;
}

void Map::visitEntities (IEntityVisitor *visitor, const EntityType& type)
{
	if (type == EntityType::NONE || type == EntityTypes::PLAYER) {
		bool needUpdate = false;
		for (PlayerListIter i = _players.begin(); i != _players.end();) {
			Player* e = *i;
			if (visitor->visitEntity(e)) {
				debug(LOG_SERVER, String::format("remove player by visit %i: %s", e->getID(), e->getType().name.c_str()));
				GameEvent.removeEntity(e->getVisMask(), *e);
				delete *i;
				i = _players.erase(i);
				needUpdate = true;
			} else {
				++i;
			}
		}
		if (needUpdate) {
			updateVisMask();
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
				debug(LOG_SERVER, String::format("remove entity by visit %i: %s", e->getID(), e->getType().name.c_str()));
				GameEvent.removeEntity(e->getVisMask(), *e, EntityTypes::isNpcCave(e->getType()));
				(*i)->prepareRemoval();
				delete *i;
				i = _entities.erase(i);
			} else {
				++i;
			}
		} else {
			++i;
		}
	}

	// now we will add the newly added entities to the list to not invalidate the iterators
	for (Map::EntityListIter i = _entitiesToAdd.begin(); i != _entitiesToAdd.end(); ++i) {
		_entities.push_back(*i);
	}
	_entitiesToAdd.clear();
}

void Map::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;

	LUA lua;

	if (!lua.load("entities.lua")) {
		System.exit("could not load entities.lua script", 1);
	}

	info(LOG_SERVER, "initialize entity sizes");

	EntityType::TypeMapConstIter i = EntityType::begin();
	for (; i != EntityType::end(); ++i) {
		String name = i->second->name;
		name = name.replaceAll("-", "");
		const float width = lua.getFloatValue(name + ".width", 1.0f);
		const float height = lua.getFloatValue(name + ".height", 1.0f);
		i->second->setSize(width, height);
	}
}
