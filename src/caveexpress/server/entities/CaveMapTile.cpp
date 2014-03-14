#include "CaveMapTile.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"

CaveMapTile::CaveMapTile (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const EntityType& npcType, int delaySpawn) :
		MapTile(map, spriteID, gridX, gridY, EntityTypes::CAVE), _nextSpawn(delaySpawn), _spawned(0), _shouldSpawnNPC(
				false), _now(0), _npc(nullptr), _lightState(DEFAULT_LIGHT_STATE), _respawn(false), _returnToCaveOnIdle(
				false), _platformStart(0), _platformEnd(0), _delaySpawn(delaySpawn)
{
	if (EntityTypes::isNpcCave(npcType))
		_npcTypes.push_back(&npcType);
}

CaveMapTile::~CaveMapTile ()
{
}

void CaveMapTile::update (uint32_t deltaTime)
{
	MapTile::update(deltaTime);

	_shouldSpawnNPC = false;
	_now += deltaTime;

	if (_npc != nullptr) {
		if (_npc->isDying()) {
			_npc = nullptr;
			_nextSpawn = _now + _delaySpawn;
		}
	}

	const int delta = _nextSpawn - _now;
	if (delta <= 0 && _respawn) {
		if (_npc != nullptr || isUnderWater())
			return;
		const Map::PlayerList& players = _map.getPlayers();
		for (Map::PlayerListConstIter i = players.begin(); i != players.end(); ++i) {
			const Player* p = *i;
			const float distance = b2Distance(p->getPos(), getPos());
			const float allowedDistance = getSize().y;
			if (distance < allowedDistance)
				return;
		}
		_shouldSpawnNPC = true;
	}
}

void CaveMapTile::setPlatformDimensions (int start, int end)
{
	_platformStart = start;
	_platformEnd = end;
}

void CaveMapTile::setRespawnPossible (bool respawn, const EntityType& type)
{
	_respawn = respawn;
	if (respawn) {
		if (EntityTypes::isNpcCave(type))
			_npcTypes.push_back(&type);
		_nextSpawn = _now + _delaySpawn;
	}
	setLightStates(_respawn);
}

bool CaveMapTile::moveBackIntoCave ()
{
	if (_npc == nullptr)
		return false;

	if (!_npc->isIdle())
		return false;

	if (!_npc->returnToInitialPosition())
		return false;

	info(LOG_SERVER, "move npc back into cave");
	setRespawnPossible(true, _npc->getType());
	_npc = nullptr;
	return true;
}

void CaveMapTile::spawnNPC ()
{
	const EntityType& type = !_npcTypes.empty() ? *_npcTypes.front() : EntityType::NONE;
	NPCPackage* npc = _map.createPackageNPC(this, type);
	if (npc == nullptr)
		return;

	if (EntityTypes::isNpcCave(type))
		_npcTypes.erase(_npcTypes.begin());
	_npc = npc;
	_spawned = _now;
	setRespawnPossible(!_npcTypes.empty(), EntityType::NONE);
}

void CaveMapTile::setLightStates (bool state)
{
	for (WindowTiles::iterator i = _windows.begin(); i != _windows.end(); ++i) {
		WindowTile* window = *i;
		window->setLightState(state);
	}
	setLightState(state);
}

void CaveMapTile::setLightState (bool lightState)
{
	const bool old = _lightState;
	_lightState = lightState;
	if (old == _lightState)
		return;

	GameEvent.sendLightState(getVisMask(), getID(), _lightState);
}
