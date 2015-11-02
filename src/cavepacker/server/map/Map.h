#pragma once

#include <memory>
#include "common/IMap.h"
#include "common/TimeManager.h"
#include "common/ICommand.h"
#include "network/IProtocolHandler.h"
#include "cavepacker/server/entities/IEntity.h"
#include "cavepacker/server/entities/Player.h"
#include "cavepacker/server/entities/MapTile.h"
#include "cavepacker/server/map/BoardState.h"
#include <string>
#include <vector>
#include <map>

// forward decl
class SpriteDef;
class IFrontend;
class ServiceProvider;
class IMapContext;

namespace cavepacker {

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

class Map: public IMap, public IEntityVisitor {
public:
	typedef std::vector<Player*> PlayerList;
	typedef PlayerList::iterator PlayerListIter;
	typedef PlayerList::const_iterator PlayerListConstIter;

	typedef std::vector<IEntity*> EntityList;
	typedef EntityList::iterator EntityListIter;
	typedef EntityList::const_iterator EntityListConstIter;

protected:
	int _height;
	int _width;

	uint32_t _restartDue;
	BoardState _state;
	typedef std::vector<IEntity*> FieldMap;
	typedef FieldMap::iterator FieldMapIter;
	typedef FieldMap::const_iterator FieldMapConstIter;
	FieldMap _field;

	// the time that passed since this map was started (milliseconds)
	uint32_t _time;

	// in a multiplayer game this is the spawn queue
	PlayerList _playersWaitingForSpawn;
	// these are the already spawned players
	PlayerList _players;

	EntityList _entities;

	bool _pause;
	// sanity check in the world step callbacks
	bool _entityRemovalAllowed;
	bool _mapRunning;

	IFrontend *_frontend;

	ServiceProvider *_serviceProvider;

	TimeManager _timeManager;

	uint16_t _moves;
	uint16_t _pushes;

	bool _forcedFinish;

	bool _autoSolve;
	int32_t _nextSolveStep;
	std::string _solution;
	std::vector<int> _deadLocks;

	bool visitEntity (IEntity *entity) override;

	// do the spawning on the map and add the physic objects
	bool spawnPlayer (Player* player);
	bool setField (IEntity *entity, int col, int row);
	void printMap ();
	char getSokobanFieldId (const IEntity *entity) const;
	void handleAutoSolve (uint32_t deltaTime);
	void checkDeadlock ();
public:
	Map ();
	virtual ~Map ();

	const PlayerList& getPlayers () const;
	inline int getConnectedPlayers () const { return _playersWaitingForSpawn.size() + _players.size(); }
	Player* getPlayer (ClientId clientId);

	void rebuildField ();
	TimeManager& getTimeManager ();

	inline bool isAutoSolve () const { return _autoSolve; }
	void abortAutoSolve ();
	std::string getMapString() const;
	inline std::string getStateString() const { return _state.toString(); }

	bool isAt(IEntity* entity, int index) const;

	int getMaxPlayers() const;
	inline int getMoves() const { return _moves; }
	inline int getPushes() const { return _pushes; }
	// return the best known moves from the solution
	inline int getBestMoves () const { return string::toInt(getSetting("best")); }
	void increaseMoves ();
	void increasePushes ();
	void undo (Player* player);

	void autoStart ();
	void loadDelayed (uint32_t delay, const std::string& name);
	bool load (const std::string& name);

	uint16_t getPoints () const;
	void addPoints (const IEntity* entity, uint16_t points);

	// move into directions l,r,d,u (sokoban standard)
	bool movePlayer (Player* player, char step);
	void moveTo (Player* player, int col, int row);

	void reload ();
	bool isFailed () const;
	bool isPause () const;
	const BoardState& getBoardState() const;

	inline bool isFree(int col, int row) {
		return _state.isFree(col, row);
	}

	void sendDeadlocks(ClientId clientId);

	// IMap
	void update (uint32_t deltaTime) override;
	bool isActive () const override;
	void restart (uint32_t delay) override;
	int getMapWidth () const override;
	int getMapHeight () const override;

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

	// checks the winning conditions of the map
	bool isDone () const;

	bool isRestartInitialized () const;

	// returns the time that was needed to finish the map
	uint32_t getTime () const;

	MapTile* getPackage (int col, int row) const;

	bool undoPackage (int col, int row, int targetCol, int targetRow);
	int getPositionIndex (IEntity* entity) const;
	char getDirectionForMove (int currentIndex, int targetIndex) const;

	void resetCurrentMap ();

	// initial add
	void loadEntity (IEntity *entity);

	void removeEntity (int clientMask, const IEntity& entity) const;
	void addEntity (int clientMask, const IEntity& entity) const;
	void updateEntity (int clientMask, const IEntity& entity) const;

	void sendMapToClient (ClientId clientId) const;

	bool removePlayer (ClientId clientId);

	void sendSound (int clientMask, const SoundType& type, const b2Vec2& pos = b2Vec2_zero) const;

	IFrontend *getFrontend () const;

	void init (IFrontend *frontend, ServiceProvider& serviceProvider);

	void shutdown ();
	int solve ();
	static std::string getSolution(const std::string& name);
private:
	void solveMap () { solve(); }
	void finishMap ();

	// command callbacks
	void triggerRestart ();
	void triggerPause ();
	void triggerDebug ();
};

inline uint32_t Map::getTime () const
{
	return _time;
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

inline TimeManager& Map::getTimeManager ()
{
	return _timeManager;
}

inline bool Map::isRestartInitialized () const
{
	return _restartDue > 0;
}

inline bool Map::isPause () const
{
	return _pause;
}

inline const BoardState& Map::getBoardState () const
{
	return _state;
}

}
