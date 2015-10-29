#pragma once

#include "caveexpress/server/entities/MapTile.h"
#include "caveexpress/server/entities/npcs/NPC.h"
#include "caveexpress/server/entities/WindowTile.h"
#include "caveexpress/server/map/Map.h"

namespace caveexpress {

// forward decl
class INPCCave;

class CaveMapTile: public MapTile {
private:
	int _nextSpawn;
	uint32_t _spawned;
	bool _shouldSpawnNPC;
	uint32_t _now;

	INPCCave* _npc;
	typedef std::vector<const EntityType*> NPCTypes;
	NPCTypes _npcTypes;
	typedef std::vector<WindowTile*> WindowTiles;
	WindowTiles _windows;
	bool _lightState;
	bool _respawn;
	bool _returnToCaveOnIdle;

	int _platformStart;
	int _platformEnd;

	// ms that each (re-)spawn is delayed
	int _delaySpawn;

	int _caveNumber;

	void setLightStates (bool state);

public:
	CaveMapTile (Map& map, int caveNumber, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const EntityType& npcType = EntityType::NONE, int delaySpawn = 5000);
	virtual ~CaveMapTile ();

	bool shouldSpawnNPC () const;
	// if spawnPackage is set to true, the npc is not collectable, but only spawns a package
	void spawnNPC (bool spawnPackage);

	void setPlatformDimensions (int start, int end);
	// the grid coordinate of the landing spots left side
	int getPlatformStartGridX () const;
	// the grid coordinate of the landing spots right side
	int getPlatformEndGridX () const;
	void setRespawnPossible (bool respawn, const EntityType& type);
	void setNextSpawn (uint32_t time);
	void setReturnToCaveOnIdle (bool returnToCaveOnIdle);

	// returns true if the light of a cave is activated, false otherwise
	bool getLightState () const;
	void setLightState (bool lightState);
	void addWindow (WindowTile* window);

	bool moveBackIntoCave ();

	int getCaveNumber() const;
	uint32_t getSpawnTime () const;

	INPCCave* getNPC () const;

	// IEntity
	void update (uint32_t deltaTime) override;
};

inline void CaveMapTile::setReturnToCaveOnIdle (bool returnToCaveOnIdle)
{
	_returnToCaveOnIdle = returnToCaveOnIdle;
}

inline void CaveMapTile::setNextSpawn (uint32_t time)
{
	_nextSpawn = time;
}

inline bool CaveMapTile::shouldSpawnNPC () const
{
	return _shouldSpawnNPC;
}

inline INPCCave* CaveMapTile::getNPC () const
{
	return _npc;
}

inline bool CaveMapTile::getLightState () const
{
	return _lightState;
}

inline void CaveMapTile::addWindow (WindowTile* window)
{
	return _windows.push_back(window);
}

inline uint32_t CaveMapTile::getSpawnTime () const
{
	return _spawned;
}

inline int CaveMapTile::getPlatformStartGridX () const
{
	return _platformStart;
}

inline int CaveMapTile::getCaveNumber() const
{
	return _caveNumber;
}

inline int CaveMapTile::getPlatformEndGridX () const
{
	return _platformEnd;
}

}
