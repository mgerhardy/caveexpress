#pragma once

#include "caveexpress/server/entities/MapTile.h"
#include "caveexpress/server/entities/npcs/NPC.h"
#include "caveexpress/server/entities/WindowTile.h"
#include "caveexpress/server/map/Map.h"

// forward decl
class NPCPackage;

class CaveMapTile: public MapTile {
private:
	int _nextSpawn;
	uint32_t _spawned;
	bool _shouldSpawnNPC;
	uint32_t _now;

	NPCPackage* _npc;
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

	void setLightStates (bool state);

public:
	CaveMapTile (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const EntityType& npcType = EntityType::NONE, int delaySpawn = 5000);
	virtual ~CaveMapTile ();

	bool isUnderWater () const;
	bool shouldSpawnNPC () const;
	void spawnNPC ();

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

	uint32_t getSpawnTime () const;

	NPCPackage* getNPC () const;

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

inline bool CaveMapTile::isUnderWater () const
{
	return _map.getWaterHeight() < getPos().y;
}

inline bool CaveMapTile::shouldSpawnNPC () const
{
	return _shouldSpawnNPC;
}

inline NPCPackage* CaveMapTile::getNPC () const
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

inline int CaveMapTile::getPlatformEndGridX () const
{
	return _platformEnd;
}
