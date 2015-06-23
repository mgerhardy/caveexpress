#include "CaveMapTile.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "caveexpress/server/entities/npcs/NPCFriendly.h"
#include "common/Log.h"

namespace caveexpress {

CaveMapTile::CaveMapTile (Map& map, int caveNumber, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const EntityType& npcType, int delaySpawn) :
		MapTile(map, spriteID, gridX, gridY, EntityTypes::CAVE), _nextSpawn(delaySpawn), _spawned(0), _shouldSpawnNPC(
				false), _now(0), _npc(nullptr), _lightState(DEFAULT_LIGHT_STATE), _respawn(false), _returnToCaveOnIdle(
				false), _platformStart(0), _platformEnd(0), _delaySpawn(delaySpawn), _caveNumber(caveNumber)
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
		if (_npc->isNpcFriendly() && _npc->isCollected()) {
			Log::info(LOG_SERVER, "npc %i is collected, remove from world", _npc->getID());
			_map.removeNPCFromWorld(static_cast<NPCFriendly*>(_npc));
			_npc = nullptr;
			_nextSpawn = _now + _delaySpawn;
		} else if (_npc->isDying()) {
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

	if (shouldSpawnNPC()) {
		const bool spawnPackage = _map.hasPackageTarget() && _map.countPackages() < _map.getPackageCount();
		if (!spawnPackage && _map.getNpcCount() <= 0)
			return;
		spawnNPC(spawnPackage);
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

	if (!_npc->isDeliverPackage()) {
		const NPCFriendly* npcFriendly = static_cast<NPCFriendly*>(_npc);
		const uint32_t waitTime = npcFriendly->getWaitPatience();
		const uint32_t passedTime = _time - getSpawnTime();
		if (passedTime < waitTime)
			return false;
	}

	if (!_npc->returnToInitialPosition())
		return false;

	Log::info(LOG_SERVER, "move npc back into cave");
	setRespawnPossible(true, _npc->getType());
	_npc = nullptr;
	return true;
}

void CaveMapTile::spawnNPC (bool spawnPackage)
{
	const EntityType& type = !_npcTypes.empty() ? *_npcTypes.front() : EntityType::NONE;
	INPCCave* npc = nullptr;
	if (spawnPackage)
		npc = _map.createPackageNPC(this, type);

	if (npc == nullptr)
		npc = _map.createFriendlyNPC(this, type, _returnToCaveOnIdle);

	if (npc == nullptr)
		return;

	if (EntityTypes::isNpcCave(type))
		_npcTypes.erase(_npcTypes.begin());
	Log::info(LOG_SERVER, "created new npc %i on cave %i", npc->getID(), _caveNumber);
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

}
