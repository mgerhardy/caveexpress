#pragma once

#include "common/Pointers.h"
#include "server/box2d/DebugRenderer.h"
#include "caveexpress/server/entities/npcs/NPC.h"
#include "caveexpress/server/entities/Border.h"
#include "caveexpress/server/entities/MapTile.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/Water.h"
#include "common/IMap.h"
#include "common/ThemeType.h"
#include "common/MapFailedReason.h"
#include "common/TimeManager.h"
#include "common/ICommand.h"
#include "network/IProtocolHandler.h"
#include "common/LUA.h"
#include <Box2D.h>
#include <string>
#include <vector>
#include <map>

// forward decl
class NPCFlying;
class NPCAttacking;
class NPCFish;
class NPCBlowing;
class IMapContext;
class Water;
class PackageTarget;
class SpriteDef;
class IFrontend;
class ServiceProvider;
class Geyser;

const int32 MAXCONTACTPOINTS = 128;
const int32 MAXTRACEDATA = 512;

class IEntityVisitor {
public:
	virtual ~IEntityVisitor ()
	{
	}

	// returns true if the just visited entity was removed from the world
	virtual bool visitEntity (IEntity *entity)
	{
		return false;
	}
};

enum {
	BORDER_TOP,
	BORDER_LEFT,
	BORDER_RIGHT,
	BORDER_BOTTOM,
	BORDER_PLAYER_BOTTOM,

	BORDER_MAX
};

class DestructionListener: public b2DestructionListener
{
	/// Called when any joint is about to be destroyed due
	/// to the destruction of one of its attached bodies.
	void SayGoodbye (b2Joint* joint) override
	{
		IEntity *entity = static_cast<IEntity*>(joint->GetUserData());
		if (entity == nullptr)
			return;
		entity->clearJoint(joint);
	}

	/// Called when any fixture is about to be destroyed due
	/// to the destruction of its parent body.
	void SayGoodbye (b2Fixture* fixture) override
	{
		fixture->SetUserData(nullptr);
		fixture->GetBody()->SetUserData(nullptr);
	}
};

class Map: public IMap, public b2ContactListener, public b2ContactFilter, public IEntityVisitor {
public:
	typedef std::vector<Player*> PlayerList;
	typedef PlayerList::iterator PlayerListIter;
	typedef PlayerList::const_iterator PlayerListConstIter;

	typedef std::vector<IEntity*> EntityList;
	typedef EntityList::iterator EntityListIter;
	typedef EntityList::const_iterator EntityListConstIter;

	typedef std::vector<CaveMapTile*> CaveList;
	typedef CaveList::iterator CaveListIter;
	typedef CaveList::const_iterator CaveListConstIter;

	ContactPoint _points[MAXCONTACTPOINTS];
	int32_t _pointCount;

	mutable TraceData _traces[MAXTRACEDATA];
	mutable int32_t _traceCount;

	mutable std::vector<b2Vec2> _waterIntersectionPoints;
protected:
	DestructionListener _destructionListener;

	float _wind;
	float _gravity;

	int _height;
	int _width;
	int _warmupPhase;

	int _restartDue;

	// this is just a pointer to the shared ptr instance - you don't have to free this
	Water *_water;
	float _waterHeight;
	float _waterChangeSpeed;
	uint32_t _waterRisingDelay;
	uint32_t _waterFallingDelay;

	bool _activateflyingNPC;
	bool _activateFishNPC;

	NPCFlying* _flyingNPC;
	NPCFish* _fishNPC;

	uint32_t _spawnFlyingNPCTime;
	uint32_t _spawnFishNPCTime;
	uint32_t _initialGeyserDelay;

	// the time that passed since this map was started (milliseconds)
	uint32_t _time;
	// the last time when the physics was updated
	int32_t _physicsTime;
	uint32_t _nextFriendlyNPCSpawn;

	int _transferedPackages;
	int _transferedPackageLimit;

	// in a multiplayer game this is the spawn queue
	PlayerList _playersWaitingForSpawn;
	// these are the already spawned players
	PlayerList _players;
	VisMask _allPlayers;

	EntityList _entities;
	// shadow copy of new entities
	EntityList _entitiesToAdd;
	// also part of the entities list
	CaveList _caves;

	typedef std::vector<Border*> BorderList;
	BorderList _borders;

	// Box2D
	b2World* _world;

	bool _pause;
	// sanity check in the world step callbacks
	bool _entityRemovalAllowed;
	bool _mapRunning;

	IFrontend *_frontend;

	ServiceProvider *_serviceProvider;

	TimeManager _timeManager;

	uint32_t _finishPoints;
	uint32_t _referenceTime;

	uint16_t _gamePoints;

	typedef std::map<int, Platform*> PlatformXMap;
	typedef PlatformXMap::const_iterator PlatformXMapConstIter;
	typedef std::map<int, PlatformXMap> PlatformYMap;
	typedef PlatformYMap::const_iterator PlatformYMapConstIter;
	PlatformYMap _platforms;
	const ThemeType* _theme;

	void calculateVisibility (IEntity *entity) const;
	bool visitEntity (IEntity *entity) override;
	void handleVisibility (IEntity *entity, const VisMask vismask) const;
	void sendVisibleEntity (int clientMask, const IEntity *entity) const;

	// do the spawning on the map and add the physic objects
	bool spawnPlayer (Player* player);
public:
	Map ();
	virtual ~Map ();

	const PlayerList& getPlayers () const;
	inline int getConnectedPlayers () const { return _playersWaitingForSpawn.size() + _players.size(); }
	Player* getPlayer (ClientId clientId);

	b2World *getWorld () const;

	TimeManager& getTimeManager ();

	void loadDelayed (uint32_t delay, const std::string& name);
	bool load (const std::string& name);

	uint16_t getPoints () const;
	void addPoints (const IEntity* entity, uint16_t points);

	// checks whether the start IEntity can reach the end IEntity by walking
	// note that this method does not work with different y values
	// param[in] start The entity that tries to reach another entity
	// param[in] end The entity that is the target
	// param[in] startPos The grid start position of the starting entity. If -1 is given here the platform dimensions are calculated
	// param[in] endPos The grid end position of the starting entity. If -1 is given here the platform dimensions are calculated
	bool isReachableByWalking (const IEntity *start, const IEntity *end, int startPos = -1, int endPos = -1) const;
	// return true if something was hit
	bool trace (const b2Vec2& start, const b2Vec2& end, IEntity **hit) const;
	bool trace (const IEntity *start, const IEntity *end, IEntity **hit = 0) const;
	bool trace (int startGridX, int startGridY, int endGridX, int endGridY, IEntity **hit) const;

	void reload ();
	bool isFailed () const;
	const MapFailedReason& getFailReason (const Player* player) const;
	bool isPause () const;

	int handleDeadPlayers ();

	bool hasPackageTarget () const;
	float getWaterHeight () const;

	// IMap
	void update (uint32_t deltaTime) override;
	bool isActive () const override;
	void restart (uint32_t delay) override;
	int getMapWidth () const override;
	int getMapHeight () const override;

	float getWind () const;
	float getGravity () const;

	CaveMapTile* getTargetCave (const CaveMapTile* ignoreCave = nullptr) const;
	CaveMapTile *getHighestCave () const;

	PackageTarget *getPackageTarget () const;

	int countPackages () const;

	void visitEntities (IEntityVisitor *visitor, const EntityType& type = EntityType::NONE);

	const IEntity* getEntity (int16_t id) const;

	// prepare the spawning
	bool initPlayer (Player* player);
	// perform the spawning of the players that are in the spawn queue
	void startMap ();
	bool isReadyToStart () const;
	void sendPlayersList () const;
	void sendMessage (ClientId clientId, const std::string& message) const;
	void printPlayersList () const;

	void disconnect (ClientId clientId);

	// creates a new flying npc (aggressive)
	NPCFlying* createFlyingNPC (const b2Vec2& pos);

	// creates a new blow npc
	NPCBlowing* createBlowingNPC (const b2Vec2& pos, bool right, float force, float modificatorSize);

	// creates a new walking npc (aggressive)
	NPCAttacking* createAttackingNPC (const b2Vec2& pos, const EntityType& entityType, bool right = true);

	NPCFish* createFishNPC (const b2Vec2& pos);

	NPCPackage* createPackageNPC (CaveMapTile* cave, const EntityType& type);

	// increases the counter for the successfully performed transfers
	void countTransferedPackage ();
	// this is the amount of packages that you still have to deliver to the package station in order to win the game
	int getPackageCount () const;

	// checks the winning conditions of the map
	// e.g. if enough npcs were brought to their target cave, the map is done
	bool isDone () const;

	bool isRestartInitialized () const;

	// returns the time that was needed to finish the map
	uint32_t getTime () const;
	const ThemeType& getTheme () const;

	// returns the time that normally is needed to finish the map
	uint32_t getReferenceTime () const;
	uint32_t getFinishPoints () const;

	void resetCurrentMap ();

	b2Body* addToWorld (b2FixtureDef &fixtureDef, b2BodyDef &bodyDef, IEntity *entity);

	// delay add
	void addEntity (IEntity *entity);
	// initial add
	void loadEntity (IEntity *entity);

	void sendMapToClient (ClientId clientId) const;

	const b2Vec2& getPlayerPos () const;

	bool removePlayer (ClientId clientId);

	void sendSound (int clientMask, const SoundType& type, const b2Vec2& pos = b2Vec2_zero) const;

	void sendSpawnInfo (const b2Vec2& pos, const EntityType& type) const;

	bool isWaterRising () const;

	IFrontend *getFrontend () const;

	void init (IFrontend *frontend, ServiceProvider& serviceProvider);

	void shutdown ();

	static void render (void *userdata);

	// get the mins and maxs (aabb) of a platform.
	// in general this is the dimension of the area that is reachable from the cave
	// by underlying solid ground tiles.
	void getPlatformDimensions (int gridX, int gridY, int *start, int *end) const;

	std::vector<b2Vec2>& getWaterIntersectionPoints () { return _waterIntersectionPoints; }

private:
	void killPlayers ();
	void finishMap ();

	void updateVisMask();

	void handleFlyingNPC ();
	void handleFishNPC ();

	MapTile* createMapTileWithoutBody (const SpriteDefPtr& spriteDef, gridCoord gridX, gridCoord gridY, EntityAngle angle);

	// b2ContactFilter
	bool ShouldCollide (b2Fixture* fixtureA, b2Fixture* fixtureB) override;

	// b2ContactListener
	void BeginContact (b2Contact* contact) override;
	void EndContact (b2Contact* contact) override;
	void PostSolve (b2Contact* contact, const b2ContactImpulse* impulse) override;
	void PreSolve (b2Contact* contact, const b2Manifold* oldManifold) override;

	// init the map boundaries and configure the box2d stuff
	void initPhysics ();

	// init the water body
	void initWater ();

	void initCave (CaveMapTile* caveTile);
	// link related windows to the cave
	void initWindows (CaveMapTile* caveTile, int start, int end);
	// place a new sensor body into the world that marks the landing platform
	// this is called at least for every cave in a map
	Platform *getPlatform (MapTile *mapTile, int *start, int *end, gridSize offset = 0.0f);

	// cleanup on map shutdown
	void clearPhysics ();

	// command callbacks
	void triggerRestart ();
	void triggerPause ();
	void triggerDebug ();
};

inline const ThemeType& Map::getTheme () const
{
	return *_theme;
}

inline uint32_t Map::getTime () const
{
	return _time - _warmupPhase;
}

inline uint32_t Map::getReferenceTime () const
{
	return _referenceTime;
}

inline uint32_t Map::getFinishPoints () const
{
	return _finishPoints;
}

inline const Map::PlayerList& Map::getPlayers () const
{
	return _players;
}

inline int Map::getMapWidth () const
{
	return _width;
}

inline int Map::getMapHeight () const
{
	return _height;
}

inline float Map::getWind () const
{
	return _wind;
}

inline float Map::getGravity () const
{
	return _gravity;
}

inline float Map::getWaterHeight () const
{
	if (_water)
		return _water->getHeight();

	return 0.0f;
}

inline b2World *Map::getWorld () const
{
	return _world;
}

inline bool Map::isDone () const
{
	if (isFailed())
		return false;
	if (_transferedPackageLimit > 0 && _transferedPackages < _transferedPackageLimit)
		return false;
	return true;
}

inline const b2Vec2& Map::getPlayerPos () const
{
	if (_players.empty())
		return b2Vec2_zero;
	return _players[0]->getPos();
}

inline bool Map::isWaterRising () const
{
	return _water->isWaterRisingEnabled();
}

inline IFrontend *Map::getFrontend () const
{
	return _frontend;
}

inline void Map::reload ()
{
	// make a copy - no reference here
	const std::string currentName = getName();
	load(currentName);
}

inline bool Map::hasPackageTarget () const
{
	for (EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		if ((*i)->isPackageTarget())
			return true;
	}
	return false;
}

inline TimeManager& Map::getTimeManager ()
{
	return _timeManager;
}

inline bool Map::isRestartInitialized () const
{
	return _restartDue > 0;
}

inline uint16_t Map::getPoints () const
{
	return _gamePoints;
}

inline bool Map::isPause () const
{
	return _pause;
}
