#include "Map.h"
#include "caveexpress/server/entities/IEntity.h"
#include "common/ConfigManager.h"
#include "common/ConfigVar.h"
#include "common/MapSettings.h"
#include "common/Shared.h"
#include "caveexpress/shared/constants/Commands.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include "caveexpress/shared/constants/Density.h"
#include "common/Log.h"
#include "network/messages/UpdateTransferCountMessage.h"
#include "service/ServiceProvider.h"
#include "common/SpriteDefinition.h"
#include "network/messages/LoadMapMessage.h"
#include "common/IFrontend.h"
#include "network/INetwork.h"
#include "common/IMapContext.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/PackageTarget.h"
#include "caveexpress/server/entities/Tree.h"
#include "caveexpress/server/entities/EntityEmitter.h"
#include "caveexpress/server/entities/WindowTile.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/server/map/RandomMapContext.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/server/entities/modificators/WindModificator.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/Geyser.h"
#include "caveexpress/server/entities/npcs/NPCAggressive.h"
#include "caveexpress/server/entities/npcs/NPCBlowing.h"
#include "caveexpress/server/entities/npcs/NPCFish.h"
#include "caveexpress/server/entities/npcs/NPCFlying.h"
#include "caveexpress/server/entities/npcs/NPCFriendly.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/CaveExpressMapFailedReasons.h"
#include "network/messages/UpdatePointsMessage.h"
#include "network/messages/UpdatePackageCountMessage.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressAchievement.h"
#include "network/messages/CooldownMessage.h"
#include "network/messages/InitDoneMessage.h"
#include "network/messages/SoundMessage.h"
#include "network/messages/MapSettingsMessage.h"
#include "network/messages/TextMessage.h"
#include "network/messages/SpawnInfoMessage.h"
#include "common/CommandSystem.h"
#include "common/System.h"
#include "common/vec2.h"
#include "common/ExecutionTime.h"
#include "common/Commands.h"
#include <SDL.h>
#include <SDL_stdinc.h>
#include <algorithm>
#include <functional>
#include <SDL_assert.h>

namespace caveexpress {

#define SPAWN_FRIENDLY_NPC_DELAY 10000
#define SPAWN_FLYING_NPC_DELAY 5000
#define SPAWN_FISH_NPC_DELAY 5000

namespace {
Achievement* packageAchievements[] = {
		&Achievements::DELIVER_A_PACKAGE,
		&Achievements::DELIVER_10_PACKAGES,
		&Achievements::DELIVER_50_PACKAGES,
		&Achievements::DELIVER_100_PACKAGES,
		&Achievements::DELIVER_150_PACKAGES
};
}

Map::Map () :
		IMap(), _world(nullptr), _frontend(nullptr), _serviceProvider(nullptr), _theme(&ThemeTypes::ROCK)
{
	Commands.registerCommandVoid(CMD_MAP_PAUSE, bindFunctionVoid(Map::triggerPause));
	Commands.registerCommandVoid(CMD_MAP_RESTART, bindFunctionVoid(Map::triggerRestart));
	Commands.registerCommandVoid(CMD_MAP_DEBUG, bindFunctionVoid(Map::triggerDebug));
	Commands.registerCommandVoid(CMD_MAP_DUMP, bindFunctionVoid(Map::dump));
	Commands.registerCommandVoid(CMD_START, bindFunctionVoid(Map::startMap));
	Commands.registerCommandVoid(CMD_KILL, bindFunctionVoid(Map::killPlayers));
	Commands.registerCommandVoid(CMD_FINISHMAP, bindFunctionVoid(Map::finishMap));

	resetCurrentMap();
}

Map::~Map ()
{
	Commands.removeCommand(CMD_MAP_PAUSE);
	Commands.removeCommand(CMD_MAP_RESTART);
	Commands.removeCommand(CMD_MAP_DEBUG);
	Commands.removeCommand(CMD_MAP_DUMP);
	Commands.removeCommand(CMD_START);
	Commands.removeCommand(CMD_KILL);
	Commands.removeCommand(CMD_FINISHMAP);
	clearPhysics();
}

void Map::finishMap ()
{
#ifdef DEBUG
	const int n = getPackageCount();
	for (int i = 0; i < n; ++i) {
		countTransferedPackage();
	}
#endif
}

void Map::killPlayers ()
{
#ifdef DEBUG
	const Map::PlayerList& players = getPlayers();
	for (Map::PlayerListConstIter i = players.begin(); i != players.end(); ++i) {
		Player* player = *i;
		player->setCrashed(CRASH_DAMAGE);
	}
#endif
}

void Map::shutdown ()
{
	resetCurrentMap();
}

void Map::addKilledNPC ()
{
	++_friendlyNPCKilled;
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

void Map::sendCooldown (int clientMask, const Cooldown& cooldown) const
{
	_serviceProvider->getNetwork().sendToClients(clientMask, CooldownMessage(cooldown));
}

void Map::sendSound (int clientMask, const SoundType& type, const PhysicsVec2& pos) const
{
	const SoundMessage msg(pos.x, pos.y, type);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::sendSpawnInfo (const PhysicsVec2& pos, const EntityType& type) const
{
	const SpawnInfoMessage msg(pos.x, pos.y, type);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void Map::updateVisMask ()
{
	_allPlayers = 0;

	for (const Player* player : _players) {
		const ClientId id = player->getClientId();
		_allPlayers |= ClientIdToClientMask(id);
	}

	for (const Player* player : _playersWaitingForSpawn) {
		const ClientId id = player->getClientId();
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

	Log::info(LOG_GAMEIMPL, "trigger restart");
	Commands.executeCommandLine(CMD_MAP_START " " + getName());
}

void Map::dump ()
{
	if (!_world)
		return;

	_world->dump();
}

void Map::triggerDebug ()
{
	const bool newstate = Config.getDebugRenderer().activate ^ true;
	Config.setDebugRenderer(newstate, render, this);
	Log::info(LOG_GAMEIMPL, "debug rendering: %s", newstate ? "true" : "false");
}

void Map::triggerPause ()
{
	if (!_serviceProvider->getNetwork().isServer())
		return;
	_pause ^= true;
	GameEvent.notifyPause(_pause);
	Log::info(LOG_GAMEIMPL, "pause: %s", _pause ? "true" : "false");
}

void Map::render (void *userdata)
{
	Map* map = static_cast<Map*>(userdata);
	if (map->_world) {
		DebugRenderer renderer(map->_pointCount, map->_points, map->_traceCount, map->_traces,  map->_waterIntersectionPoints, Config.getMapDebugRect(), map->_frontend);
		map->_world->setDebugDraw(&renderer);
		map->_world->debugDraw();
		map->_world->setDebugDraw(nullptr);
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

void Map::countTransferedNPC()
{
	_transferedNPCs++;
	Log::info(LOG_GAMEIMPL, "collected %i of %i npcs", _transferedNPCs, _transferedNPCLimit);

	const UpdateTransferCountMessage msg(
		_transferedNPCs, _transferedNPCLimit);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void Map::countTransferedPackage ()
{
	const int n = SDL_arraysize(packageAchievements);
	for (int i = 0; i < n; ++i) {
		packageAchievements[i]->unlock();
	}
	_transferedPackages++;
	Log::info(LOG_GAMEIMPL, "collected %i of %i packages", _transferedPackages, _transferedPackageLimit);
	
	const UpdatePackageCountMessage msg(
		_transferedPackages, _transferedPackageLimit);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

int Map::getNpcCount() const
{
	return _transferedNPCLimit - _transferedNPCs;
}

int Map::getPackageCount () const
{
	return _transferedPackageLimit - _transferedPackages;
}

void Map::clearPhysics ()
{
	if (!_name.empty())
		Log::info(LOG_GAMEIMPL, "* clear physics");

	if (_world)
		_world->setContactListener(nullptr);
	{ // delete the box2d stuff
		for (IEntity* entity : _entities) {
			entity->prepareRemoval();
		}
		for (Player* player : _players) {
			player->prepareRemoval();
		}
		for (IEntity* entity : _entitiesToAdd) {
			entity->prepareRemoval();
		}
		if (!_name.empty()) {
			Log::info(LOG_GAMEIMPL, "* removed box2d references");
		}
	}

	{ // now free the allocated memory
		for (Border* border : _borders) {
			delete border;
		}
		_borders.clear();

		for (IEntity* entity : _entities) {
			delete entity;
		}
		for (IEntity* entity : _entitiesToAdd) {
			delete entity;
		}
		_entitiesToAdd.clear();
		_entities.clear();
		_caves.clear();
		_platforms.clear();
		_entities.reserve(800);
		_friendlyNPCs.clear();

		for (Player* player : _players) {
			delete player;
		}
		_players.clear();
		_players.reserve(MAX_CLIENTS);
		if (!_name.empty()) {
			Log::info(LOG_GAMEIMPL, "* removed allocated memory");
		}
	}

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		delete *i;
	}
	_playersWaitingForSpawn.clear();
	_playersWaitingForSpawn.reserve(MAX_CLIENTS);

	if (_world)
		delete _world;
	if (!_name.empty())
		Log::info(LOG_GAMEIMPL, "* removed box2d world");
	_world = nullptr;
	_water = nullptr;
	_flyingNPC = nullptr;
	_fishNPC = nullptr;
}

Player* Map::getPlayer (ClientId clientId)
{
	for (Player* player : _players) {
		if (player->getClientId() == clientId) {
			return player;
		}
	}

	for (Player* player : _playersWaitingForSpawn) {
		if (player->getClientId() == clientId) {
			return player;
		}
	}

	Log::error(LOG_GAMEIMPL, "no player found for the client id %i", clientId);
	return nullptr;
}

bool Map::isFailed () const
{
	if (getWaterHeight() <= 0) {
		Log::debug(LOG_GAMEIMPL, "failed because water hit the top");
		return true;
	}

	if (_players.empty())
		return true;

	if (_friendlyNPCLimit > 0) {
		// if we support friendly npcs in this map, and all of them are (or were) already spawned,
		// but none is available anymore, this map is lost
		if (_friendlyNPCCount >= _friendlyNPCLimit) {
			if (_friendlyNPCKilled >= _friendlyNPCCount)
			{
				Log::warn(LOG_GAMEIMPL, "failed because of all NPCs killed....");
				return true;
			}
		}
	}

	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
		const Player* player = *i;
		if (!player->isCrashed()) {
			return false;
		}
	}

	Log::debug(LOG_GAMEIMPL, "failed because all %i players crashed", (int)_players.size());
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

	if (_transferedNPCLimit > 0 && _friendlyNPCLimit <= 0) {
		return MapFailedReasons::FAILED_ALL_NPCS_DIED;
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
		Log::info(LOG_GAMEIMPL, "player %s is dead", p->getName().c_str());
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

	Log::info(LOG_GAMEIMPL, "trigger map restart");
	_finishPending = false;
	_restartDue = _time + delay;
	GameEvent.restartMap(delay);
}

void Map::scheduleFinish (uint32_t delay)
{
	if (_restartDue > 0)
		return;

	Log::info(LOG_GAMEIMPL, "trigger map finish delay %u ms", delay);
	_finishPending = true;
	_restartDue = _time + delay;
	// Reuse restartMap so the client fades out with the existing overlay.
	GameEvent.restartMap(delay);
}

void Map::resetCurrentMap ()
{
	_timeManager.reset();
	if (!_name.empty()) {
		GameEvent.closeMap();
		Log::info(LOG_GAMEIMPL, "reset map: %s", _name.c_str());
	}
	_pointCount = 0;
	_traceCount = 0;
	_gamePoints = 0;
	_finishPoints = 0;
	_referenceTime = 0;
	_warmupPhase = 0;
	_restartDue = 0;
	_finishPending = false;
	_pause = false;
	_transferedNPCs = 0;
	_transferedNPCLimit = 0;
	_transferedPackages = 0;
	_transferedPackageLimit = 0;
	_nextFriendlyNPCSpawn = 0;
	_spawnFlyingNPCTime = 0;
	_activateflyingNPC = false;
	_friendlyNPCLimit = 0;
	_friendlyNPCCount = 0;
	_friendlyNPCKilled = 0;
	_caveCounter = 0;
	_spawnFishNPCTime = 0;
	_initialGeyserDelay = 0;
	_activateFishNPC = false;
	_mapRunning = false;
	_wind = 0.0f;
	_width = 0;
	_height = 0;
	_gravity = 0.0f;
	_flyingSpeedX = 1.0f;
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
		Log::info(LOG_GAMEIMPL, "done with resetting: %s", _name.c_str());
	_name.clear();
}

inline const ThemeType& getTheme (const std::string& name)
{
	const std::size_t themeSep = name.find("-");
	if (themeSep == std::string::npos)
		return ThemeTypes::ROCK;
	const std::string &themeName = name.substr(themeSep + 1, name.size() - (themeSep + 1));
	const ThemeType& t = ThemeType::getByName(themeName);
	if (!t.isNone())
		return t;
	return ThemeTypes::ROCK;
}

inline CaveExpressMapContext* getMapContext (const std::string& name)
{
	const std::string randomMapBase = "random";
	if (name.compare(0, randomMapBase.size(), randomMapBase) == 0) {
		const ThemeType& theme = getTheme(name);
		Log::info(LOG_GAMEIMPL, "use theme %s", theme.name.c_str());
		RandomMapContext *ctx = new RandomMapContext(name, theme, 8, 18, 20, 14);
		return ctx;
	}
	return new CaveExpressMapContext(name);
}

bool Map::load (const std::string& name)
{
	std::unique_ptr<CaveExpressMapContext> ctx(getMapContext(name));

	resetCurrentMap();

	if (name.empty()) {
		Log::info(LOG_GAMEIMPL, "no map name given");
		return false;
	}

	Log::info(LOG_GAMEIMPL, "load map %s", name.c_str());

	if (!ctx->load(false)) {
		Log::error(LOG_GAMEIMPL, "failed to load the map %s", name.c_str());
		return false;
	}
	//ctx->save();
	_settings = ctx->getSettings();
	_startPositions = ctx->getStartPositions();
	_name = ctx->getName();
	_title = ctx->getTitle();
	_theme = &ctx->getTheme();
	_settings.insert(std::make_pair(msn::THEME, _theme->name));
	_wind = string::toFloat(getSetting(msn::WIND, msd::WIND));
	_gravity = string::toFloat(getSetting(msn::GRAVITY, string::toString(msdv::GRAVITY)));
	_flyingSpeedX = Config.getConfigVar(FLYING_SPEED_X)->getFloatValue();
	_width = string::toInt(getSetting(msn::WIDTH, "-1"));
	_height = string::toInt(getSetting(msn::HEIGHT, "-1"));
	_finishPoints = string::toInt(getSetting(msn::POINTS, string::toString(msdv::POINTS)));
	_referenceTime = string::toInt(getSetting(msn::REFERENCETIME, string::toString(msdv::REFERENCETIME)));
	_waterChangeSpeed = string::toFloat(getSetting(msn::WATER_CHANGE, msd::WATER_CHANGE));
	_waterRisingDelay = string::toFloat(getSetting(msn::WATER_RISING_DELAY, msd::WATER_RISING_DELAY));
	_waterFallingDelay = string::toFloat(getSetting(msn::WATER_FALLING_DELAY, msd::WATER_FALLING_DELAY));
	_transferedNPCLimit = string::toInt(getSetting(msn::NPC_TRANSFER_COUNT, msd::NPC_TRANSFER_COUNT));
	_friendlyNPCLimit = string::toInt(getSetting(msn::NPCS, msd::NPCS));
	_activateflyingNPC = string::toBool(getSetting(msn::FLYING_NPC, msd::FLYING_NPC));
	_activateFishNPC = string::toBool(getSetting(msn::FISH_NPC, msd::FISH_NPC));
	_waterHeight = string::toFloat(getSetting(msn::WATER_HEIGHT, msd::WATER_HEIGHT));
	_transferedPackageLimit = string::toInt(getSetting(msn::PACKAGE_TRANSFER_COUNT, msd::PACKAGE_TRANSFER_COUNT));
	// TODO: properly implement a warmup phase
	_warmupPhase = 0;

	Log::info(LOG_GAMEIMPL, "spawn %i npcs", _friendlyNPCLimit);
	Log::info(LOG_GAMEIMPL, "theme: %s", _theme->name.c_str());
	Log::info(LOG_GAMEIMPL, "reference time: %u", _referenceTime);

	if (_width <= 0 || _height <= 0) {
		Log::error(LOG_GAMEIMPL, "invalid map dimensions given");
		return false;
	}

	if (_transferedNPCLimit > 0 && _friendlyNPCLimit == 0) {
		Log::error(LOG_GAMEIMPL, "there is no npc but a npc transfer count");
		return false;
	}

	_spawnFlyingNPCTime = string::toInt(getSetting(msn::NPC_INITIAL_SPAWN_TIME, string::toString(4000 + rand() % SPAWN_FLYING_NPC_DELAY)));
	_spawnFishNPCTime = string::toInt(getSetting(msn::NPC_INITIAL_SPAWN_TIME, string::toString(4000 + rand() % SPAWN_FISH_NPC_DELAY)));
	_initialGeyserDelay = string::toInt(getSetting(msn::GEYSER_INITIAL_DELAY_TIME, string::toString(3000)));

	if (_transferedNPCLimit <= 0 && _transferedPackageLimit <= 0) {
		Log::error(LOG_GAMEIMPL, "there is nothing to do in this map - set the npc or package limits");
		return false;
	}

	initPhysics();
	Log::info(LOG_GAMEIMPL, "physics initialized");

	std::vector<MapTile*> mapTilesWithBody;

	const std::vector<MapTileDefinition>& mapTileList = ctx->getMapTileDefinitions();
	for (const MapTileDefinition& mapTileDef : mapTileList) {
		MapTile *mapTile = createMapTileWithoutBody(mapTileDef.spriteDef, mapTileDef.x, mapTileDef.y, mapTileDef.angle);
		if (!mapTile->isDecoration() && !mapTile->isWindow()) {
			mapTilesWithBody.push_back(mapTile);
		}
		loadEntity(mapTile);
	}

	const std::vector<CaveTileDefinition>& caveList = ctx->getCaveTileDefinitions();
	for (const CaveTileDefinition& caveTileDef : caveList) {
		const SpriteDefPtr &spriteDef = caveTileDef.spriteDef;
		MapTile *mapTile = new CaveMapTile(*this, ++_caveCounter, spriteDef->id, caveTileDef.x, caveTileDef.y, *caveTileDef.type, caveTileDef.delay);
		mapTile->setGridDimensions(spriteDef->width, spriteDef->height, 0);
		if (loadEntity(mapTile)) {
			mapTilesWithBody.push_back(mapTile);
		}
	}

	for (MapTile* mapTile : mapTilesWithBody) {
		mapTile->createBody();
	}

	Log::info(LOG_GAMEIMPL, "init platforms");
	std::vector<MapTile*> platforms;
	for (IEntity* entity : _entities) {
		if (!entity->isGround()) {
			continue;
		}
		MapTile *mapTile = assert_cast<MapTile*, IEntity*>(entity);
		platforms.push_back(mapTile);
	}
	for (MapTile* mapTile : platforms) {
		int start = -1;
		int end = -1;
		const int y = mapTile->getGridY() - 1.0f + EPSILON;
		getPlatformDimensions(mapTile->getGridX(), y, &start, &end);
		if (start == -1 || end == -1) {
			continue;
		}
		getPlatform(mapTile, &start, &end);
	}

	Log::info(LOG_GAMEIMPL, "init caves");
	int friendlyNPCLimit = _friendlyNPCLimit;
	for (IEntity* entity : _entities) {
		if (!entity->isCave()) {
			continue;
		}
		CaveMapTile *cave = assert_cast<CaveMapTile*, IEntity*>(entity);
		_caves.push_back(cave);
	}

	const CaveMapTile* highestCave = nullptr;
	if (isWaterRising() && !_water->isWaterFallingEnabled()) {
		highestCave = getHighestCave();
	}

	// do another loop when we have all caves - we have to know each of the caves in order to initialize them properly
	for (CaveMapTile* cave : _caves) {
		const bool npcLeft = friendlyNPCLimit > 0;
		const bool skipCave = highestCave == cave;
		if (initCave(cave, npcLeft && !skipCave)) {
			--friendlyNPCLimit;
			Log::info(LOG_GAMEIMPL, "spawn npc on cave %i", cave->getCaveNumber());
		}
	}
	if (friendlyNPCLimit > 0) {
		Log::info(LOG_GAMEIMPL, "could not spawn %i npcs", friendlyNPCLimit);
	}

	const std::vector<EmitterDefinition>& emitterList = ctx->getEmitterDefinitions();
	for (const EmitterDefinition &emitterDef : emitterList) {
		const EntityType &type = *emitterDef.type;
		if (type.isNone()) {
			continue;
		}
		EntityEmitter *entity = new EntityEmitter(*this, emitterDef.x, emitterDef.y, emitterDef.amount, emitterDef.delay, type, emitterDef.settings);
		loadEntity(entity);
	}

	if (_transferedPackageLimit > 0 && !hasPackageTarget()) {
		Log::error(LOG_GAMEIMPL, "there is no package target in this map");
		return false;
	}
	if (_transferedPackageLimit <= 0 && hasPackageTarget()) {
		Log::error(LOG_GAMEIMPL, "transferpackagecount is not set, but there are package targets");
		return false;
	}

	Log::info(LOG_GAMEIMPL, "map loading done");

	ctx->onMapLoaded();

	_frontend->onMapLoaded();
	const LoadMapMessage msg(_name, _title);
	_serviceProvider->getNetwork().sendToClients(0, msg);

	_mapRunning = true;
	return true;
}

class TraceCallback: public IPhysicsRayCastCallback {
private:
	float _fraction;
	IEntity *_entity;

public:
	TraceCallback () :
			_fraction(0.0), _entity(nullptr)
	{
	}

	float reportFixture (PhysicsFixture fixture, const PhysicsVec2& point, const PhysicsVec2& normal, float fraction) override
	{
		IEntity *e = reinterpret_cast<IEntity *>(fixture.getBody().getUserData());
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
	rayTrace(start, end, &entity);
	if (entity != nullptr && entity->isSolid()) {
		return false;
	}

	if (startPos == -1 || endPos == -1) {
		getPlatformDimensions(static_cast<int>(start->getPos().x), static_cast<int>(start->getPos().y), &startPos, &endPos);
	}

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

bool Map::rayTrace (const PhysicsVec2& start, const PhysicsVec2& end, IEntity **hit) const
{
	TraceCallback callback;
	_world->rayCast(callback, start, end);
	if (hit) {
		*hit = callback.getEntity();
	}

	uint32_t index = _traceCount;
	if (index < SDL_arraysize(_traces)) {
		_traces[index].start = start;
		_traces[index].end = end;
		_traces[index].fraction = callback.getFraction();
		_traceCount++;
	}

	return callback.getFraction() < 1.0f;
}

bool Map::rayTrace (const IEntity *start, const IEntity *end, IEntity **hit) const
{
	return rayTrace(start->getPos(), end->getPos(), hit);
}

bool Map::rayTrace (int startGridX, int startGridY, int endGridX, int endGridY, IEntity **hit) const
{
	// center of the cells
	const PhysicsVec2 start(startGridX + 0.5f, startGridY + 0.5f);
	const PhysicsVec2 end(endGridX + 0.5f, endGridY + 0.5f);
	return rayTrace(start, end, hit);
}

bool Map::spawnPlayer (Player* player)
{
	SDL_assert(_entityRemovalAllowed);

	Log::info(LOG_GAMEIMPL, "spawn player %i", player->getID());
	const int startPosIdx = _players.size();
	float playerStartX, playerStartY;
	if (!getStartPosition(startPosIdx, playerStartX, playerStartY)) {
		Log::error(LOG_GAMEIMPL, "no player position for index %i", startPosIdx);
		return false;
	}

	const PhysicsVec2& size = player->getSize();
	const PhysicsVec2 pos(playerStartX + size.x / 2.0f, playerStartY + size.y / 2.0f);
	player->createBody(pos);
	player->onSpawn();
	_players.push_back(player);
	return true;
}

void Map::sendMessage (ClientId clientId, const std::string& message) const
{
	INetwork& network = _serviceProvider->getNetwork();
	network.sendToClient(clientId, TextMessage(message));
}

bool Map::isReadyToStart () const
{
	return _playersWaitingForSpawn.size() > 1;
}

void Map::startMap ()
{
	Log::info(LOG_GAMEIMPL, "start the map and spawn pending players: %i", (int)_playersWaitingForSpawn.size());
	for (Player* player : _playersWaitingForSpawn) {
		spawnPlayer(player);
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

	SDL_assert(_entityRemovalAllowed);

	INetwork& network = _serviceProvider->getNetwork();
	const ClientId clientId = player->getClientId();
	Log::info(LOG_GAMEIMPL, "init player %i", player->getID());
	const int clientMask = ClientIdToClientMask(clientId);
	const MapSettingsMessage mapSettingsMsg(_settings, _startPositions.size());
	network.sendToClient(clientId, mapSettingsMsg);
	GameEvent.sendWaterUpdate(clientMask, *_water);

	const InitDoneMessage msgInit(player->getID(),
		getPackageCount(), getNpcCount(), player->getLives(), player->getHitpoints());
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
	Log::info(LOG_GAMEIMPL, "delay spawn of player");
	_playersWaitingForSpawn.push_back(player);
	return true;
}

void Map::printPlayersList () const
{
	for (Player* player : _playersWaitingForSpawn) {
		const std::string& name = player->getName();
		Log::info(LOG_GAMEIMPL, "* %s (waiting)", name.c_str());
	}
	for (Player* player : _players) {
		const std::string& name = player->getName();
		Log::info(LOG_GAMEIMPL, "* %s (spawned)", name.c_str());
	}
}

void Map::sendPlayersList () const
{
	std::vector<std::string> names;
	for (Player* player : _players) {
		const std::string& name = player->getName();
		names.push_back(name);
	}
	for (Player* player : _playersWaitingForSpawn) {
		const std::string& name = player->getName();
		names.push_back(name);
	}
	INetwork& network = _serviceProvider->getNetwork();
	network.sendToAllClients(PlayerListMessage(names));
}

void Map::initPhysics ()
{
	PhysicsVec2 gravity;
	gravity.set(0.0f, getGravity());
	_world = new PhysicsWorld(gravity);

	_world->setDestructionListener(&_destructionListener);

	//_world->SetWarmStarting(false);
	//_world->SetContinuousPhysics(false);
	//_world->SetSubStepping(false);
	_world->setAutoClearForces(true);
	_world->setContactListener(this);
	_world->setContactFilter(this);

	const float zeroX = 0.0f;
	const float zeroY = -0.5f;
	const float width = getMapWidth();
	// added a small offset to allow water diving out of screen
	const float height = getMapHeight() + 1.0f;

	PhysicsBodyDef lineBodyDef;
	lineBodyDef.type = PhysicsBodyType::Static;
	lineBodyDef.position.set(0, 0);
	PhysicsBody boxBody = _world->createBody(lineBodyDef);

	PhysicsFixtureDef fd;
	fd.shapeType = PhysicsShapeType::Edge;
	fd.vertexCount = 2;
	fd.friction = 1.0f;
	fd.restitution = 0.2f;

	_borders.resize(BORDER_MAX);
	const bool isSideBorderFail = string::toBool(getSetting(msn::SIDEBORDERFAIL));
	_borders[BORDER_TOP] = new Border(BorderType::TOP, *this);
	_borders[BORDER_LEFT] = new Border(BorderType::LEFT, *this, isSideBorderFail);
	_borders[BORDER_RIGHT] = new Border(BorderType::RIGHT, *this, isSideBorderFail);
	_borders[BORDER_BOTTOM] = new Border(BorderType::BOTTOM, *this);
	_borders[BORDER_PLAYER_BOTTOM] = new Border(BorderType::PLAYER_BOTTOM, *this);

	fd.vertices[0].set(zeroX, zeroY);
	fd.vertices[1].set(width, zeroY);
	fd.userData = (PhysicsUserData)_borders[BORDER_TOP];
	boxBody.createFixture(fd);

	fd.vertices[0].set(zeroX, zeroY);
	fd.vertices[1].set(zeroX, height);
	fd.userData = (PhysicsUserData)_borders[BORDER_LEFT];
	boxBody.createFixture(fd);

	fd.vertices[0].set(width, height);
	fd.vertices[1].set(width, zeroY);
	fd.userData = (PhysicsUserData)_borders[BORDER_RIGHT];
	boxBody.createFixture(fd);

	fd.vertices[0].set(zeroX, height);
	fd.vertices[1].set(width, height);
	fd.userData = (PhysicsUserData)_borders[BORDER_BOTTOM];
	boxBody.createFixture(fd);

	fd.vertices[0].set(zeroX, height);
	fd.vertices[1].set(width, getMapHeight());
	fd.userData = (PhysicsUserData)_borders[BORDER_PLAYER_BOTTOM];
	boxBody.createFixture(fd);

	initWater();
}

bool Map::initCave (CaveMapTile* caveTile, bool canSpawn)
{
	int start = -1;
	int end = -1;
	Platform *platform = getPlatform(caveTile, &start, &end, caveTile->getSize().y);
	if (platform == nullptr) {
		Log::error(LOG_GAMEIMPL, "failed to initialize the cave platform");
		return false;
	}
	platform->setCave(caveTile);
	initWindows(caveTile, start, end);
	caveTile->setRespawnPossible(canSpawn, EntityType::NONE);
	caveTile->setPlatformDimensions(start, end);
	return canSpawn;
}

void Map::initWindows (CaveMapTile* caveTile, int start, int end)
{
	const int x = (int)caveTile->getGridX();
	const int left = std::max(x - 2, start);
	const int right = std::max(x + 2, end);
	const int caveY = (int)caveTile->getGridY();
	for (IEntity* e : _entities) {
		if (!e->isWindow()) {
			continue;
		}
		WindowTile* window = assert_cast<WindowTile*, IEntity*>(e);
		if ((int)window->getGridY() != caveY) {
			continue;
		}
		for (int gridX = left; gridX < right; ++gridX) {
			const int windowX = (int)window->getGridX();
			if (Between(windowX, left, right)) {
				caveTile->addWindow(window);
				break;
			}
		}
	}
}

Platform *Map::getPlatform (MapTile *mapTile, int *start, int *end, gridSize offset)
{
	const int mapY = (int)(mapTile->getGridY() + offset + EPSILON);
	if (*start == -1 || *end == -1) {
		getPlatformDimensions((int)mapTile->getGridX(), (int)mapTile->getGridY(), start, end);
	}

	PlatformYMapConstIter iy = _platforms.find(mapY);
	if (iy != _platforms.end()) {
		PlatformXMapConstIter ix = iy->second.find(*start);
		if (ix != iy->second.end()) {
			return ix->second;
		}
	}

	Log::info(LOG_GAMEIMPL, "create a new platform at %i:%i to %i:%i", *start, mapY, *end, mapY);
	const int width = *end - *start + 1;
	const gridSize height = 0.015f;
	const gridCoord x = (gridCoord)*start + (gridCoord)width / 2.0f;
	const gridSize y = mapTile->getGridY() + offset;

	PhysicsFixtureDef fixture;
	fixture.useBox = true;
	fixture.boxHalfWidth = width / 2.0f;
	fixture.boxHalfHeight = height;
	fixture.friction = 0.4f;
	fixture.restitution = 0.0f;
	fixture.density = 0.0f;

	PhysicsBodyDef bd;
	bd.position.set(x, y);
	bd.type = PhysicsBodyType::Kinematic;
	bd.fixedRotation = true;

	Platform *platform = new Platform(*this);
	addToWorld(fixture, bd, platform);
	loadEntity(platform);

	_platforms[mapY][*start] = platform;

#ifdef DEBUG
	PlatformYMapConstIter iy2 = _platforms.find(mapY);
	SDL_assert(iy2 != _platforms.end());
	if (iy != _platforms.end()) {
		PlatformXMapConstIter ix2 = iy2->second.find(*start);
		SDL_assert(ix2 != iy->second.end());
	}
#endif

	return platform;
}

void Map::getPlatformDimensions (int gridX, int startTraceGridY, int *start, int *end) const
{
	const int endTraceGridY = startTraceGridY + 1;

	if (gridX > 0) {
		IEntity *hit = nullptr;
		rayTrace(gridX, startTraceGridY, 0, startTraceGridY, &hit);
		int leftGridX = 0;
		if (hit && hit->isSolid()) {
			MapTile *mapTile = assert_cast<MapTile*, IEntity*>(hit);
			leftGridX = mapTile->getGridX() + mapTile->getGridWidth();
		}
		int startTraceGridX = gridX;
		const int steps = startTraceGridX - leftGridX;
		for (int i = 0; i < steps; ++i) {
			const PhysicsVec2 startV(startTraceGridX - 0.5f, startTraceGridY + 0.5f);
			const PhysicsVec2 endV(startTraceGridX - 0.5f, endTraceGridY + 0.2f);
			const bool state = rayTrace(startV, endV, &hit);
			if (state && hit && hit->isSolid()) {
				--startTraceGridX;
				continue;
			}
			break;
		}
		*start = startTraceGridX;
	} else {
		IEntity *hit = nullptr;
		const PhysicsVec2 startV(0, startTraceGridY);
		const PhysicsVec2 endV(0, startTraceGridY + 0.0001f);
		const bool state = rayTrace(startV, endV, &hit);
		if (state && hit && hit->isSolid())
			return;

		*start = 0;
	}

	if (gridX < _width - 1) {
		IEntity *hit = nullptr;
		rayTrace(gridX, startTraceGridY, _width - 1, startTraceGridY, &hit);
		int rightGridX = _width - 1;
		if (hit && hit->isSolid()) {
			MapTile *mapTile = assert_cast<MapTile*, IEntity*>(hit);
			// we always subtract 1 here - not the width - it's the left side
			rightGridX = mapTile->getGridX() - 1;
		}
		int endTraceGridX = gridX;
		const int steps = rightGridX - endTraceGridX;
		for (int i = 0; i < steps; ++i) {
			const PhysicsVec2 startV(endTraceGridX + 1.5f, startTraceGridY + 0.5f);
			const PhysicsVec2 endV(endTraceGridX + 1.5f, endTraceGridY + 0.2f);
			const bool state = rayTrace(startV, endV, &hit);
			if (state && hit && hit->isSolid()) {
				++endTraceGridX;
				continue;
			}
			break;
		}
		*end = endTraceGridX;
	} else {
		IEntity *hit = nullptr;
		const PhysicsVec2 startV(_width - 1.0f, startTraceGridY);
		const PhysicsVec2 endV(_width - 1.0f, startTraceGridY + 0.0001f);
		const bool state = rayTrace(startV, endV, &hit);
		if (state && hit && hit->isSolid()) {
			return;
		}

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
		mapTile = new CaveMapTile(*this, ++_caveCounter, spriteDef->id, gridX, gridY);
	} else if (SpriteTypes::isBackground(type)) {
		mapTile = new MapTile(*this, spriteDef->id, gridX, gridY, EntityTypes::DECORATION);
	} else if (SpriteTypes::isWindow(type)) {
		mapTile = new WindowTile(*this, spriteDef->id, gridX, gridY);
	} else if (SpriteTypes::isLava(type)) {
		mapTile = new MapTile(*this, spriteDef->id, gridX, gridY, EntityTypes::LAVA);
	} else if (SpriteTypes::isPackageTarget(type)) {
		mapTile = new PackageTarget(*this, spriteDef->id, gridX, gridY);
	} else if (SpriteTypes::isGeyser(type)) {
		mapTile = new Geyser(*this, spriteDef->id, gridX, gridY, _initialGeyserDelay);
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

PhysicsBody Map::addToWorld (PhysicsFixtureDef &fixtureDef, PhysicsBodyDef &bodyDef, IEntity *entity)
{
	SDL_assert(_entityRemovalAllowed);

	SpriteDefPtr def = entity->getSpriteDef();
	bool useProvidedShape = fixtureDef.useBox || fixtureDef.vertexCount > 0
			|| (fixtureDef.shapeType == PhysicsShapeType::Circle && fixtureDef.radius > 0.0f)
			|| fixtureDef.shapeType == PhysicsShapeType::Edge;
	if (def) {
		if (def->hasShape())
			useProvidedShape = false;
		fixtureDef.restitution = def->restitution;
		fixtureDef.friction = def->friction;
	}

	bodyDef.userData = (PhysicsUserData)entity;
	PhysicsBody body = _world->createBody(bodyDef);

	if (useProvidedShape) {
		fixtureDef.userData = (PhysicsUserData)const_cast<char*>("");
		body.createFixture(fixtureDef);
		entity->addBody(body);
		return body;
	}

	if (!def) {
		Log::error(LOG_GAMEIMPL, "no shape given - could not find sprite definition for %s", entity->getType().name.c_str());
		return PhysicsBody();
	}

	// create the shape from the sprite definition polygons
	const int polygons = def->polygons.size();
	for (int j = 0; j < polygons; ++j) {
		const SpritePolygon& polygon = def->polygons[j];
		const int cnt = polygon.vertices.size();
		int vertexCnt = 0;
		const int size = PhysicsMaxPolygonVertices;
		if (cnt > size)
			Log::error(LOG_GAMEIMPL, "too many vertices given for sprite %s", def->id.c_str());

		PhysicsFixtureDef polyFd = fixtureDef;
		polyFd.shapeType = PhysicsShapeType::Polygon;
		polyFd.useBox = false;
		polyFd.radius = 0.0f;
		for (int i = 0; i < cnt; ++i) {
			const SpriteVertex &v = polygon.vertices[i];
			polyFd.vertices[vertexCnt].x = v.x;
			polyFd.vertices[vertexCnt].y = v.y * -1.0f;
			vertexCnt++;
			if (vertexCnt >= size)
				break;
		}

		if (vertexCnt == 0)
			continue;

		polyFd.vertexCount = vertexCnt;
		polyFd.userData = (PhysicsUserData)const_cast<char*>(polygon.userData.c_str());
		body.createFixture(polyFd);
	}

	// create the shape from the sprite definition circles
	for (std::vector<SpriteCircle>::const_iterator i = def->circles.begin(); i != def->circles.end(); ++i) {
		const SpriteCircle& circle = *i;
		PhysicsFixtureDef circleFd = fixtureDef;
		circleFd.shapeType = PhysicsShapeType::Circle;
		circleFd.useBox = false;
		circleFd.vertexCount = 0;
		circleFd.circleCenter = PhysicsVec2(circle.center.x, circle.center.y);
		circleFd.radius = circle.radius;
		circleFd.userData = (PhysicsUserData)const_cast<char*>(circle.userData.c_str());
		body.createFixture(circleFd);
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

bool Map::loadEntity (IEntity *entity)
{
	SDL_assert(_entityRemovalAllowed);
	//entity->onSpawn();
	SDL_assert(entity != nullptr);
#if 0
	if (_entities.size() + 1 > _entities.capacity()) {
		Log::error(LOG_SERVER, "entity list is full - cannot add more entities");
		delete entity;
		return false;
	}
#endif
	_entities.push_back(entity);
	return true;
}

PackageTarget *Map::getPackageTarget () const
{
	std::vector<const PackageTarget*> packageTargets;
	for (EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		if (!(*i)->isPackageTarget())
			continue;
		const PackageTarget *packageTarget = assert_cast<const PackageTarget*, const IEntity*>(*i);
		packageTargets.push_back(packageTarget);
	}
	const int packageTargetCount = packageTargets.size();
	if (packageTargetCount == 0) {
		Log::info(LOG_GAMEIMPL, "no package target found");
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
		Log::debug(LOG_GAMEIMPL, "no usable cave found");
		return nullptr;
	}

	const int randomCave = rand() % tmp.size();
	return tmp[randomCave];
}

bool Map::removeNPCFromWorld(NPCFriendly* npc)
{
	SDL_assert(_entityRemovalAllowed);
	Log::debug(LOG_GAMEIMPL, "remove npc %i from world: %s", npc->getID(), npc->getType().name.c_str());
	GameEvent.removeEntity(npc->getVisMask(), *npc);
	npc->setVisMask(NOTVISIBLE);
	npc->remove();
	return true;

}

bool Map::removeNPC(NPCFriendly* npc, bool fadeOut)
{
	SDL_assert(_entityRemovalAllowed);
	for (Map::NPCListIter i = _friendlyNPCs.begin(); i != _friendlyNPCs.end(); ++i) {
		if (*i != npc)
			continue;

		Log::info(LOG_GAMEIMPL, "remove friendly npc %i: %s", npc->getID(), npc->getType().name.c_str());
		_friendlyNPCs.erase(i);
		GameEvent.removeEntity(npc->getVisMask(), *npc, fadeOut);
		npc->setVisMask(NOTVISIBLE);
		npc->remove();
		_nextFriendlyNPCSpawn = _time + rand() % SPAWN_FRIENDLY_NPC_DELAY;
		// they are also part of the entities list and are freed there
		return true;
	}
	Log::error(LOG_GAMEIMPL, "could not find the npc with the id %i", npc->getID());
	return false;
}

bool Map::removePlayer (ClientId clientId)
{
	SDL_assert(_entityRemovalAllowed);

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
			NPCAttacking *npcAttacking = assert_cast<NPCAttacking*, IEntity*>(*refIter);
			npcAttacking->stopAttack(*i);
		}

		GameEvent.removeEntity((*i)->getVisMask(), **i);
		(*i)->prepareRemoval();
		delete *i;
		_players.erase(i);
		updateVisMask();
		return true;
	}
	Log::error(LOG_GAMEIMPL, "could not find the player with the clientId %i", clientId);
	return false;
}

NPCBlowing* Map::createBlowingNPC (const PhysicsVec2& pos, bool right, float force, float modificatorSize)
{
	SDL_assert(_entityRemovalAllowed);

	NPCBlowing *npc = new NPCBlowing(*this, pos, right, force, modificatorSize);
	addEntity(npc);

	return npc;
}

NPCAttacking* Map::createAttackingNPC (const PhysicsVec2& pos, const EntityType& entityType, bool right)
{
	SDL_assert(EntityTypes::isNpcAttacking(entityType));
	SDL_assert(_entityRemovalAllowed);
	NPCAttacking *npc = new NPCAttacking(entityType, *this, right);
	npc->createBody(pos);
	npc->calculatePlatformDimensions();
	addEntity(npc);
	return npc;
}

NPCFish* Map::createFishNPC (const PhysicsVec2& pos)
{
	SDL_assert(_entityRemovalAllowed);
	NPCFish *npc = new NPCFish(*this);
	npc->createBody(pos, false, true);
	addEntity(npc);
	return npc;
}

NPCFlying* Map::createFlyingNPC (const PhysicsVec2& pos)
{
	SDL_assert(_entityRemovalAllowed);
	NPCFlying *npc = new NPCFlying(*this);
	npc->createBody(pos);
	addEntity(npc);
	return npc;
}

NPCPackage* Map::createPackageNPC (CaveMapTile* cave, const EntityType& type)
{
	SDL_assert(_entityRemovalAllowed);

	if (getPackageTarget() == nullptr)
		return nullptr;

	NPCPackage* npc = new NPCPackage(cave, type);
	addEntity(npc);

	visitEntity(npc);

	return npc;
}

NPCFriendly* Map::createFriendlyNPC(CaveMapTile* cave, const EntityType& type, bool returnToCaveOnIdle)
{
	SDL_assert(_entityRemovalAllowed);
	if (_friendlyNPCs.size() >= _friendlyNPCLimit)
		return nullptr;

	if (_time < _nextFriendlyNPCSpawn)
		return nullptr;

	CaveMapTile* targetCave = getTargetCave(cave);
	if (targetCave == nullptr)
		return nullptr;

	NPCFriendly* npc = new NPCFriendly(cave, type, returnToCaveOnIdle);
	npc->setTargetCave(targetCave);
	_friendlyNPCs.push_back(npc);
	++_friendlyNPCCount;

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
		//Log::info(LOG_GAMEIMPL, string::format("server: remove entity %i type: %s", entity->getID(), entity->getType().name.c_str()));
		GameEvent.removeEntity(removeMask, *entity);
	}

	if (addMask != 0) {
		sendVisibleEntity(addMask, entity);
	}
}

void Map::sendVisibleEntity (int clientMask, const IEntity *entity) const
{
	//Log::debug(LOG_GAMEIMPL, string::format("server: add entity %i type: %s", entity->getID(), entity->getType().name.c_str()));
	GameEvent.addEntity(clientMask, *entity);
	if (entity->isCave()) {
		const CaveMapTile *tile = assert_cast<const CaveMapTile *, const IEntity*>(entity);
		const int caveNumber = _transferedNPCLimit > 0 ? tile->getCaveNumber() : 0;
		GameEvent.addCave(clientMask, entity->getID(), caveNumber, tile->getLightState());
	} else if (entity->isWindow()) {
		const WindowTile *tile = assert_cast<const WindowTile *, const IEntity*>(entity);
		GameEvent.sendLightState(clientMask, tile->getID(), tile->getLightState());
	}
}

bool Map::visitEntity (IEntity *entity)
{
	const VisMask vismask = entity->getVisMask();
	if (_time >= _warmupPhase) {
		entity->update(Constant::DELTA_PHYSICS_MILLIS);
		if (entity->shouldApplyWind())
			entity->applyLinearImpulse(PhysicsVec2(_wind * getFlyingSpeedX() * (DENSITY_PLAYER / 400.0f), 0.0f));
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
		const PhysicsVec2& pos = player->getPos();
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
		const PhysicsVec2 npcSpawnPos(x, y);
		_flyingNPC = createFlyingNPC(npcSpawnPos);
	} else {
		const PhysicsVec2 &pos = _flyingNPC->getPos();
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
		const PhysicsVec2& pos = player->getPos();
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
		const PhysicsVec2 npcSpawnPos(x, y);
		_fishNPC = createFishNPC(npcSpawnPos);
	} else {
		const PhysicsVec2 &pos = _fishNPC->getPos();
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

	if (_restartDue > 0 && _restartDue <= _time) {
		const std::string currentName = getName();
		const bool finishPending = _finishPending;
		_restartDue = 0;
		_finishPending = false;
		if (finishPending && !isFailed()) {
			Log::info(LOG_GAMEIMPL, "map finish delay elapsed for %s", currentName.c_str());
			return;
		}
		Log::info(LOG_GAMEIMPL, "restarting map %s", currentName.c_str());
		if (isFailed()) {
			const Map::PlayerList& players = getPlayers();
			for (Map::PlayerListConstIter i = players.begin(); i != players.end(); ++i) {
				const Player* p = *i;
				GameEvent.failedMap(p->getClientId(), currentName, getFailReason(p), getTheme());
			}
			System.track("mapstate", "failed:" + currentName);
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
				_world->step(Constant::DELTA_PHYSICS_SECONDS, 8, 3);
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

bool Map::shouldCollide (PhysicsFixture fixtureA, PhysicsFixture fixtureB)
{
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA.getUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB.getUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		const bool shouldCollide = entity1->shouldCollide(entity2) || entity2->shouldCollide(entity1);
		if (entity1->shouldRefilter())
			fixtureA.refilter();
		if (entity2->shouldRefilter())
			fixtureB.refilter();
		return shouldCollide;
	}

	const PhysicsFilter& filterA = fixtureA.getFilterData();
	const PhysicsFilter& filterB = fixtureB.getFilterData();
	if (filterA.groupIndex == filterB.groupIndex && filterA.groupIndex != 0) {
		return filterA.groupIndex > 0;
	}

	const bool collide = (filterA.maskBits & filterB.categoryBits) != 0 && (filterA.categoryBits & filterB.maskBits) != 0;
	return collide;
}

void Map::beginContact (PhysicsContact contact)
{
	PhysicsFixture fixtureA = contact.getFixtureA();
	PhysicsFixture fixtureB = contact.getFixtureB();
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA.getUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB.getUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		entity1->onContact(contact, entity2);
		entity2->onContact(contact, entity1);
	}
}

void Map::endContact (PhysicsContact contact)
{
	PhysicsFixture fixtureA = contact.getFixtureA();
	PhysicsFixture fixtureB = contact.getFixtureB();
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA.getUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB.getUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		entity1->endContact(contact, entity2);
		entity2->endContact(contact, entity1);
	}
}

void Map::postSolve (PhysicsContact contact, const PhysicsContactImpulse& impulse)
{
	PhysicsFixture fixtureA = contact.getFixtureA();
	PhysicsFixture fixtureB = contact.getFixtureB();
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA.getUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB.getUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		entity1->onPostSolve(contact, impulse, entity2);
		entity2->onPostSolve(contact, impulse, entity1);
	}
}

void Map::preSolve (PhysicsContact contact, const PhysicsManifold& oldManifold)
{
	PhysicsFixture fixtureA = contact.getFixtureA();
	PhysicsFixture fixtureB = contact.getFixtureB();
	IEntity *entity1 = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
	IEntity *entity2 = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());

	if (entity1 == nullptr)
		entity1 = reinterpret_cast<IEntity*>(fixtureA.getUserData());
	if (entity2 == nullptr)
		entity2 = reinterpret_cast<IEntity*>(fixtureB.getUserData());

	if (entity1 != nullptr && entity2 != nullptr) {
		entity1->onPreSolve(contact, entity2, oldManifold);
		entity2->onPreSolve(contact, entity1, oldManifold);
	}

	const PhysicsManifold manifold = contact.getManifold();

	if (manifold.pointCount == 0) {
		return;
	}

	PhysicsPointState state1[PhysicsMaxManifoldPoints], state2[PhysicsMaxManifoldPoints];
	physGetPointStates(state1, state2, oldManifold, manifold);

	PhysicsWorldManifold worldManifold;
	contact.getWorldManifold(worldManifold);

	for (int32_t i = 0; i < manifold.pointCount && _pointCount < MAXCONTACTPOINTS; ++i) {
		ContactPoint* cp = _points + _pointCount;
		cp->fixtureA = fixtureA;
		cp->fixtureB = fixtureB;
		cp->position = worldManifold.points[i];
		cp->normal = worldManifold.normal;
		cp->state = state2[i];
		cp->normalImpulse = manifold.points[i].normalImpulse;
		cp->tangentImpulse = manifold.points[i].tangentImpulse;
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
		const Package *package = assert_cast<Package*, IEntity*>(*i);
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
				Log::debug(LOG_GAMEIMPL, "remove player by visit %i: %s", e->getID(), e->getType().name.c_str());
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
				_friendlyNPCs.remove((NPCFriendly*)e);
				Log::debug(LOG_GAMEIMPL, "remove entity by visit %i: %s", e->getID(), e->getType().name.c_str());
				GameEvent.removeEntity(e->getVisMask(), *e, EntityTypes::isNpcCave(e->getType()));
				e->prepareRemoval();
				delete e;
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
		IEntity *ent = *i;
		SDL_assert(ent != nullptr);
		_entities.push_back(ent);
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

	Log::info(LOG_GAMEIMPL, "initialize entity sizes");

	EntityType::TypeMapConstIter i = EntityType::begin();
	for (; i != EntityType::end(); ++i) {
		const std::string& name = string::replaceAll(i->second->name, "-", "");
		const float width = lua.getFloatValue(name + ".width", 1.0f);
		const float height = lua.getFloatValue(name + ".height", 1.0f);
		Log::debug(LOG_GAMEIMPL, "entity %s: %f:%f", name.c_str(), width, height);
		i->second->setSize(width, height);
	}
	Log::debug(LOG_GAMEIMPL, "initialized entity sizes");
}

}
