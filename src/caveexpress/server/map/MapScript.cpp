#include "MapScript.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/server/entities/IEntity.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/PackageTarget.h"
#include "caveexpress/server/entities/CollectableEntity.h"
#include "caveexpress/server/entities/MapTile.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/npcs/NPC.h"
#include "caveexpress/server/entities/npcs/NPCFriendly.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/server/entities/npcs/NPCFlying.h"
#include "caveexpress/server/entities/npcs/NPCFish.h"
#include "caveexpress/server/entities/npcs/NPCBlowing.h"
#include "caveexpress/server/entities/npcs/INPCCave.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "common/ConfigManager.h"
#include "common/Direction.h"
#include "common/Log.h"
#include "common/LUALibrary.h"
#include "common/String.h"
#include "common/Animation.h"
#include "common/Common.h"
#include <SDL.h>
#include <cstring>

namespace caveexpress {

namespace {

CaveExpressMapContext* luaGetMapContext (lua_State* L, int n = 1)
{
	return LUA::getUserData<CaveExpressMapContext>(L, n, "Map");
}

Map* luaRequireMap (lua_State* L, int n = 1)
{
	CaveExpressMapContext* ctx = luaGetMapContext(L, n);
	Map* map = ctx != nullptr ? ctx->getRuntimeMap() : nullptr;
	if (map == nullptr)
		luaL_error(L, "map runtime is not available (only valid in onMapLoaded/onUpdate)");
	return map;
}

struct LuaEntityRef {
	Map* map;
	uint16_t id;
};

void luaPushEntity (lua_State* L, IEntity* entity)
{
	if (entity == nullptr) {
		lua_pushnil(L);
		return;
	}
	LuaEntityRef* ref = static_cast<LuaEntityRef*>(lua_newuserdata(L, sizeof(LuaEntityRef)));
	ref->map = &entity->getMap();
	ref->id = entity->getID();
	luaL_getmetatable(L, "luaL_Entity");
	lua_setmetatable(L, -2);
}

LuaEntityRef* luaGetEntityRef (lua_State* L, int n = 1)
{
	return static_cast<LuaEntityRef*>(luaL_checkudata(L, n, "luaL_Entity"));
}

IEntity* luaResolveEntity (lua_State* L, int n = 1, bool required = true)
{
	LuaEntityRef* ref = luaGetEntityRef(L, n);
	if (ref->map == nullptr) {
		if (required)
			luaL_error(L, "invalid entity");
		return nullptr;
	}
	IEntity* entity = ref->map->findEntity(ref->id);
	if (entity == nullptr || entity->isRemove()) {
		if (required)
			luaL_error(L, "entity %u is no longer valid", static_cast<unsigned>(ref->id));
		return nullptr;
	}
	return entity;
}

IEntity* luaGetEntity (lua_State* L, int n = 1)
{
	return luaResolveEntity(L, n, true);
}

CaveMapTile* luaGetCaveEntity (lua_State* L, int n = 1)
{
	IEntity* entity = luaGetEntity(L, n);
	if (!entity->isCave())
		luaL_error(L, "entity is not a cave");
	return assert_cast<CaveMapTile*, IEntity*>(entity);
}

Direction luaParseDirection (lua_State* L, int index)
{
	if (lua_isnumber(L, index))
		return static_cast<Direction>(luaL_checkinteger(L, index));
	const char* name = luaL_checkstring(L, index);
	if (!SDL_strcasecmp(name, "left"))
		return DIRECTION_LEFT;
	if (!SDL_strcasecmp(name, "right"))
		return DIRECTION_RIGHT;
	if (!SDL_strcasecmp(name, "up"))
		return DIRECTION_UP;
	if (!SDL_strcasecmp(name, "down"))
		return DIRECTION_DOWN;
	luaL_error(L, "unknown direction '%s' (use left/right/up/down)", name);
	return 0;
}

const EntityType& luaParseEntityType (lua_State* L, int index, const EntityType& fallback = EntityType::NONE)
{
	if (lua_isnoneornil(L, index))
		return fallback;
	const std::string name = luaL_checkstring(L, index);
	const EntityType& type = EntityType::getByName(name);
	if (type.isNone() && !name.empty() && SDL_strcasecmp(name.c_str(), "none") != 0)
		luaL_error(L, "unknown entity type '%s'", name.c_str());
	return type.isNone() ? fallback : type;
}

bool isPhysicalKeyPressed (const std::string& name)
{
	const Uint8* state = SDL_GetKeyboardState(nullptr);
	if (state == nullptr)
		return false;

	std::string converted = string::replaceAll(name, "_", " ");
	SDL_Keycode keycode = SDL_GetKeyFromName(converted.c_str());
	if (keycode == SDLK_UNKNOWN) {
		// also try without conversion / as scancode name
		keycode = SDL_GetKeyFromName(name.c_str());
	}
	if (keycode != SDLK_UNKNOWN) {
		const SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
		if (scancode != SDL_SCANCODE_UNKNOWN && state[scancode])
			return true;
	}

	const SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());
	if (scancode != SDL_SCANCODE_UNKNOWN && state[scancode])
		return true;

	return false;
}

bool isCommandBoundKeyPressed (const std::string& command)
{
	const std::map<int, std::string>& bindings = Config.getKeyBindings();
	const Uint8* state = SDL_GetKeyboardState(nullptr);
	if (state == nullptr)
		return false;

	for (const auto& entry : bindings) {
		const std::string& bound = entry.second;
		if (bound != command && bound != ("+" + command))
			continue;
		const SDL_Scancode scancode = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(entry.first));
		if (scancode != SDL_SCANCODE_UNKNOWN && state[scancode])
			return true;
	}
	return false;
}

bool resolveKeyPressed (const std::string& name, Map* map)
{
	// Prefer client-fed intent (works for remote clients and headless servers).
	if (map != nullptr) {
		if (!SDL_strcasecmp(name.c_str(), "skip")) {
			if (map->isScriptClientSkipPressed())
				return true;
		} else if (!SDL_strcasecmp(name.c_str(), "any")) {
			if (map->isScriptClientSkipPressed() || map->isScriptClientActionPressed()
					|| map->isScriptClientDirectionPressed(DIRECTION_LEFT)
					|| map->isScriptClientDirectionPressed(DIRECTION_RIGHT)
					|| map->isScriptClientDirectionPressed(DIRECTION_UP)
					|| map->isScriptClientDirectionPressed(DIRECTION_DOWN))
				return true;
		} else if (!SDL_strcasecmp(name.c_str(), "drop") || !SDL_strcasecmp(name.c_str(), "action")) {
			if (map->isScriptClientActionPressed())
				return true;
		} else if (!SDL_strcasecmp(name.c_str(), "left")) {
			if (map->isScriptClientDirectionPressed(DIRECTION_LEFT))
				return true;
		} else if (!SDL_strcasecmp(name.c_str(), "right")) {
			if (map->isScriptClientDirectionPressed(DIRECTION_RIGHT))
				return true;
		} else if (!SDL_strcasecmp(name.c_str(), "up")) {
			if (map->isScriptClientDirectionPressed(DIRECTION_UP))
				return true;
		} else if (!SDL_strcasecmp(name.c_str(), "down")) {
			if (map->isScriptClientDirectionPressed(DIRECTION_DOWN))
				return true;
		}
	}

	if (isPhysicalKeyPressed(name))
		return true;

	// Convenience aliases used in map scripts (local keyboard fallback)
	if (!SDL_strcasecmp(name.c_str(), "drop") || !SDL_strcasecmp(name.c_str(), "action"))
		return isCommandBoundKeyPressed("drop") || isPhysicalKeyPressed("Space") || isPhysicalKeyPressed("Return");
	if (!SDL_strcasecmp(name.c_str(), "left"))
		return isCommandBoundKeyPressed("+move_left") || isCommandBoundKeyPressed("move_left") || isPhysicalKeyPressed("Left") || isPhysicalKeyPressed("A");
	if (!SDL_strcasecmp(name.c_str(), "right"))
		return isCommandBoundKeyPressed("+move_right") || isCommandBoundKeyPressed("move_right") || isPhysicalKeyPressed("Right") || isPhysicalKeyPressed("D");
	if (!SDL_strcasecmp(name.c_str(), "up"))
		return isCommandBoundKeyPressed("+move_up") || isCommandBoundKeyPressed("move_up") || isPhysicalKeyPressed("Up") || isPhysicalKeyPressed("W");
	if (!SDL_strcasecmp(name.c_str(), "down"))
		return isCommandBoundKeyPressed("+move_down") || isCommandBoundKeyPressed("move_down") || isPhysicalKeyPressed("Down") || isPhysicalKeyPressed("S");
	// Skip is Escape (and client-latched Space/action) — never fly keys.
	if (!SDL_strcasecmp(name.c_str(), "skip"))
		return isPhysicalKeyPressed("Escape");
	if (!SDL_strcasecmp(name.c_str(), "any")) {
		return resolveKeyPressed("drop", map) || resolveKeyPressed("left", map) || resolveKeyPressed("right", map)
				|| resolveKeyPressed("up", map) || resolveKeyPressed("down", map) || isPhysicalKeyPressed("Escape")
				|| isPhysicalKeyPressed("Space") || isPhysicalKeyPressed("Return");
	}

	return isCommandBoundKeyPressed(name);
}

Map* tryGetRuntimeMap (lua_State* L)
{
	lua_getglobal(L, "Map");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return nullptr;
	}
	lua_getfield(L, -1, "get");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return nullptr;
	}
	if (lua_pcall(L, 0, 1, 0) != 0) {
		lua_pop(L, 2); // error + Map
		return nullptr;
	}
	if (lua_isnil(L, -1) || !lua_isuserdata(L, -1)) {
		lua_pop(L, 2);
		return nullptr;
	}
	CaveExpressMapContext* ctx = LUA::getUserData<CaveExpressMapContext>(L, -1, "Map");
	Map* map = ctx != nullptr ? ctx->getRuntimeMap() : nullptr;
	lua_pop(L, 2);
	return map;
}

// --- Map runtime methods ----------------------------------------------------

int luaMapFinish (lua_State* L)
{
	Map* map = luaRequireMap(L);
	map->forceComplete();
	return 0;
}

int luaMapIsDone (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushboolean(L, map->isDone() ? 1 : 0);
	return 1;
}

int luaMapGetTime (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushnumber(L, map->getTime());
	return 1;
}

int luaMapGetSize (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushnumber(L, map->getMapWidth());
	lua_pushnumber(L, map->getMapHeight());
	return 2;
}

int luaMapSetInputEnabled (lua_State* L)
{
	Map* map = luaRequireMap(L);
	map->setInputEnabled(lua_toboolean(L, 2) != 0);
	return 0;
}

int luaMapIsInputEnabled (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushboolean(L, map->isInputEnabled() ? 1 : 0);
	return 1;
}

int luaMapIsKeyPressed (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const char* name = luaL_checkstring(L, 2);
	lua_pushboolean(L, resolveKeyPressed(name, map) ? 1 : 0);
	return 1;
}

int luaMapMessage (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const char* text = luaL_checkstring(L, 2);
	const uint32_t delayMillis = static_cast<uint32_t>(luaL_optinteger(L, 3, 4000));
	map->broadcastScriptMessage(text, delayMillis);
	return 0;
}

int luaMapGetGravity (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushnumber(L, map->getGravity());
	return 1;
}

int luaMapConsumeSkip (lua_State* L)
{
	Map* map = luaRequireMap(L);
	map->consumeScriptClientSkip();
	return 0;
}

/**
 * Ballistic launch velocity to reach (toX,toY) from (fromX,fromY) in flightTime seconds
 * under constant gravity (positive Y down). Returns vx, vy.
 */
int luaMapCalculateVelocity (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const float fromX = static_cast<float>(luaL_checknumber(L, 2));
	const float fromY = static_cast<float>(luaL_checknumber(L, 3));
	const float toX = static_cast<float>(luaL_checknumber(L, 4));
	const float toY = static_cast<float>(luaL_checknumber(L, 5));
	const float flightTime = static_cast<float>(luaL_checknumber(L, 6));
	const float gravity = static_cast<float>(luaL_optnumber(L, 7, map->getGravity()));
	if (flightTime <= 0.0f)
		luaL_error(L, "calculateVelocity: flightTime must be > 0");
	const float vx = (toX - fromX) / flightTime;
	const float vy = (toY - fromY) / flightTime - 0.5f * gravity * flightTime;
	lua_pushnumber(L, vx);
	lua_pushnumber(L, vy);
	return 2;
}

int luaMapGetPlayer (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const int index = static_cast<int>(luaL_optinteger(L, 2, 1) - 1); // Lua is 1-based
	const Map::PlayerList& players = map->getPlayers();
	if (index < 0 || index >= (int)players.size()) {
		lua_pushnil(L);
		return 1;
	}
	luaPushEntity(L, players[index]);
	return 1;
}

int luaMapGetEntity (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const uint16_t id = static_cast<uint16_t>(luaL_checkinteger(L, 2));
	IEntity* entity = map->findEntity(id);
	if (entity == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, entity);
	return 1;
}

int luaMapGetCaveCount (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushinteger(L, map->getCaveCount());
	return 1;
}

int luaMapGetCave (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const int index = static_cast<int>(luaL_checkinteger(L, 2) - 1); // 1-based
	CaveMapTile* cave = map->getCave(index);
	if (cave == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, cave);
	return 1;
}

int luaMapGetWaterHeight (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushnumber(L, map->getWaterHeight());
	return 1;
}

int luaMapSetWaterHeight (lua_State* L)
{
	Map* map = luaRequireMap(L);
	map->setWaterHeight(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

int luaMapSpawnPackage (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const float x = static_cast<float>(luaL_checknumber(L, 2));
	const float y = static_cast<float>(luaL_checknumber(L, 3));
	Package* package = map->spawnPackageScripted(x, y);
	if (package == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, package);
	return 1;
}

int luaMapGetPackageCount (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushinteger(L, map->countPackages());
	return 1;
}

int luaMapGetPackage (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const int index = static_cast<int>(luaL_checkinteger(L, 2) - 1); // 1-based
	Package* package = map->getPackage(index);
	if (package == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, package);
	return 1;
}

int luaMapGetPackages (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_newtable(L);
	int n = 0;
	for (int i = 0; ; ++i) {
		Package* package = map->getPackage(i);
		if (package == nullptr)
			break;
		luaPushEntity(L, package);
		lua_rawseti(L, -2, ++n);
	}
	return 1;
}

int luaMapGetPackageTarget (lua_State* L)
{
	Map* map = luaRequireMap(L);
	PackageTarget* target = map->getPackageTarget();
	if (target == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, target);
	return 1;
}

int luaMapGetDeliveredPackageCount (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushinteger(L, map->getDeliveredPackageCount());
	return 1;
}

int luaMapGetCollectedPackageCount (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushinteger(L, map->getCollectedPackageCount());
	return 1;
}

int luaMapGetPackageDeliveryGoal (lua_State* L)
{
	Map* map = luaRequireMap(L);
	lua_pushinteger(L, map->getPackageDeliveryGoal());
	return 1;
}

int luaMapSpawnPackageNPC (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const int caveIndex = static_cast<int>(luaL_checkinteger(L, 2) - 1);
	CaveMapTile* cave = map->getCave(caveIndex);
	const EntityType& type = luaParseEntityType(L, 3, EntityTypes::NPC_FRIENDLY_MAN);
	NPCPackage* npc = map->spawnPackageNPCScripted(cave, type);
	if (npc == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, npc);
	return 1;
}

int luaMapSpawnFriendlyNPC (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const int caveIndex = static_cast<int>(luaL_checkinteger(L, 2) - 1);
	CaveMapTile* cave = map->getCave(caveIndex);
	const EntityType& type = luaParseEntityType(L, 3, EntityTypes::NPC_FRIENDLY_MAN);
	const bool returnToCave = lua_toboolean(L, 4) != 0;
	NPCFriendly* npc = map->spawnFriendlyNPCScripted(cave, type, returnToCave);
	if (npc == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, npc);
	return 1;
}

int luaMapSpawnNPC (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const EntityType& type = luaParseEntityType(L, 2);
	const float x = static_cast<float>(luaL_checknumber(L, 3));
	const float y = static_cast<float>(luaL_checknumber(L, 4));

	IEntity* spawned = nullptr;
	if (EntityTypes::isNpcFlying(type)) {
		spawned = map->spawnFlyingNPCScripted(x, y);
	} else if (EntityTypes::isNpcFish(type)) {
		spawned = map->spawnFishNPCScripted(x, y);
	} else if (EntityTypes::isNpcBlowing(type)) {
		const bool right = lua_toboolean(L, 5) != 0;
		const float force = static_cast<float>(luaL_optnumber(L, 6, 1.0));
		const float size = static_cast<float>(luaL_optnumber(L, 7, 1.0));
		spawned = map->spawnBlowingNPCScripted(x, y, right, force, size);
	} else if (EntityTypes::isNpcAttacking(type) || EntityTypes::isNpcWalking(type) || EntityTypes::isNpcMammut(type)) {
		const bool right = lua_isnoneornil(L, 5) ? true : (lua_toboolean(L, 5) != 0);
		spawned = map->spawnAttackingNPCScripted(x, y, type, right);
	} else {
		luaL_error(L, "spawnNPC does not support type '%s' (use spawnFriendlyNPC/spawnPackageNPC for cave NPCs)",
				type.name.c_str());
	}

	if (spawned == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, spawned);
	return 1;
}

int luaMapAddTile (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const char* sprite = luaL_checkstring(L, 2);
	const float x = static_cast<float>(luaL_checknumber(L, 3));
	const float y = static_cast<float>(luaL_checknumber(L, 4));
	const EntityAngle angle = static_cast<EntityAngle>(luaL_optinteger(L, 5, 0));
	MapTile* tile = map->addTileScripted(sprite, x, y, angle);
	if (tile == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, tile);
	return 1;
}

int luaMapRemoveTileAt (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const int x = static_cast<int>(luaL_checkinteger(L, 2));
	const int y = static_cast<int>(luaL_checkinteger(L, 3));
	lua_pushinteger(L, map->removeTileAtScripted(x, y));
	return 1;
}

int luaMapReplaceTile (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const char* sprite = luaL_checkstring(L, 2);
	const float x = static_cast<float>(luaL_checknumber(L, 3));
	const float y = static_cast<float>(luaL_checknumber(L, 4));
	const EntityAngle angle = static_cast<EntityAngle>(luaL_optinteger(L, 5, 0));
	MapTile* tile = map->replaceTileScripted(sprite, x, y, angle);
	if (tile == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, tile);
	return 1;
}

int luaMapRebuildPlatforms (lua_State* L)
{
	Map* map = luaRequireMap(L);
	map->rebuildPlatforms();
	return 0;
}

int luaMapRemoveEntity (lua_State* L)
{
	Map* map = luaRequireMap(L);
	IEntity* entity = nullptr;
	if (lua_isuserdata(L, 2)) {
		entity = luaResolveEntity(L, 2, false);
	} else {
		entity = map->findEntity(static_cast<uint16_t>(luaL_checkinteger(L, 2)));
	}
	if (entity == nullptr)
		return 0;
	if (entity->isPlayer())
		luaL_error(L, "cannot remove the player entity");
	entity->setRemove(true);
	return 0;
}

// --- Entity methods ---------------------------------------------------------

int luaEntityIsValid (lua_State* L)
{
	lua_pushboolean(L, luaResolveEntity(L, 1, false) != nullptr ? 1 : 0);
	return 1;
}

int luaEntityGetId (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	lua_pushinteger(L, entity->getID());
	return 1;
}

int luaEntityGetType (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	lua_pushstring(L, entity->getType().name.c_str());
	return 1;
}

int luaEntityGetPos (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const PhysicsVec2& pos = entity->getPos();
	lua_pushnumber(L, pos.x);
	lua_pushnumber(L, pos.y);
	return 2;
}

int luaEntitySetPos (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const float x = static_cast<float>(luaL_checknumber(L, 2));
	const float y = static_cast<float>(luaL_checknumber(L, 3));
	entity->setPos(PhysicsVec2(x, y));
	return 0;
}

int luaEntityGetVelocity (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const PhysicsVec2& v = entity->getLinearVelocity();
	lua_pushnumber(L, v.x);
	lua_pushnumber(L, v.y);
	return 2;
}

int luaEntitySetVelocity (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const float x = static_cast<float>(luaL_checknumber(L, 2));
	const float y = static_cast<float>(luaL_checknumber(L, 3));
	entity->setLinearVelocity(PhysicsVec2(x, y));
	return 0;
}

int luaEntityApplyImpulse (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const float x = static_cast<float>(luaL_checknumber(L, 2));
	const float y = static_cast<float>(luaL_checknumber(L, 3));
	entity->applyLinearImpulse(PhysicsVec2(x, y));
	return 0;
}

int luaEntityApplyForce (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const float x = static_cast<float>(luaL_checknumber(L, 2));
	const float y = static_cast<float>(luaL_checknumber(L, 3));
	entity->applyForceToCenter(PhysicsVec2(x, y));
	return 0;
}

int luaEntityGetGravityScale (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	lua_pushnumber(L, entity->getGravityScale());
	return 1;
}

int luaEntitySetGravityScale (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	entity->setGravityScale(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

int luaEntityGetGravity (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const PhysicsVec2& g = entity->getGravity();
	lua_pushnumber(L, g.x);
	lua_pushnumber(L, g.y);
	return 2;
}

int luaEntityGetState (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	lua_pushinteger(L, entity->getState());
	return 1;
}

int luaEntitySetState (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	entity->setState(static_cast<int>(luaL_checkinteger(L, 2)));
	return 0;
}

int luaEntityRemove (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (entity->isPlayer())
		luaL_error(L, "cannot remove the player entity");
	// Cave NPCs are tracked on their cave; clear that before deletion or CaveMapTile
	// will call into a dangling pointer on the next update.
	if (entity->isNpcCave()) {
		INPCCave* caveNpc = assert_cast<INPCCave*, IEntity*>(entity);
		CaveMapTile* cave = caveNpc->getCave();
		if (cave != nullptr && cave->getNPC() == caveNpc)
			cave->setNPC(nullptr);
	}
	if (entity->isNpcFriendly()) {
		NPCFriendly* friendly = assert_cast<NPCFriendly*, IEntity*>(entity);
		friendly->getMap().removeNPC(friendly, true);
		return 0;
	}
	entity->setRemove(true);
	return 0;
}

int luaEntityIsPlayer (lua_State* L)
{
	lua_pushboolean(L, luaGetEntity(L)->isPlayer() ? 1 : 0);
	return 1;
}

int luaEntityIsNpc (lua_State* L)
{
	lua_pushboolean(L, luaGetEntity(L)->isNpc() ? 1 : 0);
	return 1;
}

int luaEntityIsCave (lua_State* L)
{
	lua_pushboolean(L, luaGetEntity(L)->isCave() ? 1 : 0);
	return 1;
}

int luaEntityAccelerate (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPlayer())
		luaL_error(L, "accelerate() is only valid for the player");
	Player* player = assert_cast<Player*, IEntity*>(entity);
	player->accelerate(luaParseDirection(L, 2));
	return 0;
}

int luaEntityResetAcceleration (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPlayer())
		luaL_error(L, "resetAcceleration() is only valid for the player");
	Player* player = assert_cast<Player*, IEntity*>(entity);
	if (lua_gettop(L) >= 2)
		player->clearAcceleration(luaParseDirection(L, 2));
	else
		player->clearAcceleration(0);
	return 0;
}

int luaEntitySetMoving (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpc())
		luaL_error(L, "setMoving() is only valid for NPCs");
	NPC* npc = assert_cast<NPC*, IEntity*>(entity);
	if (lua_gettop(L) >= 3) {
		npc->setMoving(PhysicsVec2(static_cast<float>(luaL_checknumber(L, 2)), static_cast<float>(luaL_checknumber(L, 3))));
	} else {
		npc->setMoving(static_cast<gridCoord>(luaL_checknumber(L, 2)));
	}
	return 0;
}

int luaEntitySetIdle (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpc())
		luaL_error(L, "setIdle() is only valid for NPCs");
	assert_cast<NPC*, IEntity*>(entity)->setIdle();
	return 0;
}

int luaEntityIsIdle (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpc())
		luaL_error(L, "isIdle() is only valid for NPCs");
	lua_pushboolean(L, assert_cast<NPC*, IEntity*>(entity)->isIdle() ? 1 : 0);
	return 1;
}

int luaEntitySetDone (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpc())
		luaL_error(L, "setDone() is only valid for NPCs");
	assert_cast<NPC*, IEntity*>(entity)->setDone();
	return 0;
}

int luaEntityReturnToCave (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpc())
		luaL_error(L, "returnToCave() is only valid for NPCs");
	lua_pushboolean(L, assert_cast<NPC*, IEntity*>(entity)->returnToInitialPosition() ? 1 : 0);
	return 1;
}

int luaEntityLeavePackage (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpcPackage())
		luaL_error(L, "leavePackage() is only valid for package NPCs");
	Package* package = assert_cast<NPCPackage*, IEntity*>(entity)->leavePackage();
	luaPushEntity(L, package);
	return 1;
}

int luaEntityDropPackage (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpcPackage())
		luaL_error(L, "dropPackage() is only valid for package NPCs");
	Package* package = assert_cast<NPCPackage*, IEntity*>(entity)->dropPackage();
	luaPushEntity(L, package);
	return 1;
}

int luaEntitySetInvulnerable (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPlayer())
		luaL_error(L, "setInvulnerable() is only valid for the player");
	Player* player = assert_cast<Player*, IEntity*>(entity);
	player->setInvulnerable(static_cast<uint32_t>(luaL_optinteger(L, 2, 10000)));
	return 0;
}

int luaEntityDrop (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPlayer())
		luaL_error(L, "drop() is only valid for the player");
	Player* player = assert_cast<Player*, IEntity*>(entity);
	player->drop();
	return 0;
}

int luaEntityGetCollectedPackageCount (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPlayer())
		luaL_error(L, "getCollectedPackageCount() is only valid for the player");
	const Player* player = assert_cast<const Player*, const IEntity*>(entity);
	lua_pushinteger(L, player->getCollectedPackageCount());
	return 1;
}

int luaEntityGetCollectedPackages (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPlayer())
		luaL_error(L, "getCollectedPackages() is only valid for the player");
	const Player* player = assert_cast<const Player*, const IEntity*>(entity);
	lua_newtable(L);
	int n = 0;
	for (int i = 0; ; ++i) {
		Package* package = player->getCollectedPackage(i);
		if (package == nullptr)
			break;
		luaPushEntity(L, package);
		lua_rawseti(L, -2, ++n);
	}
	return 1;
}

int luaEntityIsPackage (lua_State* L)
{
	lua_pushboolean(L, luaGetEntity(L)->isPackage());
	return 1;
}

int luaEntityIsCollected (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isCollectable())
		luaL_error(L, "isCollected() is only valid for collectables");
	lua_pushboolean(L, assert_cast<CollectableEntity*, IEntity*>(entity)->isCollected());
	return 1;
}

int luaEntityIsArrived (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPackage())
		luaL_error(L, "isArrived() is only valid for packages");
	const Package* package = assert_cast<const Package*, const IEntity*>(entity);
	lua_pushboolean(L, package->isArrived());
	return 1;
}

int luaEntityIsDelivered (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPackage())
		luaL_error(L, "isDelivered() is only valid for packages");
	const Package* package = assert_cast<const Package*, const IEntity*>(entity);
	lua_pushboolean(L, package->isDelivered());
	return 1;
}

int luaEntityIsDestroyed (lua_State* L)
{
	lua_pushboolean(L, luaGetEntity(L)->isDestroyed());
	return 1;
}

int luaEntityIsPulling (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPackageTarget())
		luaL_error(L, "isPulling() is only valid for package targets");
	const PackageTarget* target = assert_cast<const PackageTarget*, const IEntity*>(entity);
	lua_pushboolean(L, target->isPulling());
	return 1;
}

int luaEntityGetPullingPackage (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isPackageTarget())
		luaL_error(L, "getPullingPackage() is only valid for package targets");
	PackageTarget* target = assert_cast<PackageTarget*, IEntity*>(entity);
	Package* package = target->getPullingPackage();
	if (package == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, package);
	return 1;
}

int luaEntitySetTargetCave (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpcFriendly())
		luaL_error(L, "setTargetCave() is only valid for friendly NPCs");
	NPCFriendly* npc = assert_cast<NPCFriendly*, IEntity*>(entity);

	if (lua_isnoneornil(L, 2)) {
		npc->setTargetCave(nullptr);
		return 0;
	}

	CaveMapTile* cave = nullptr;
	if (lua_isnumber(L, 2)) {
		Map* map = &entity->getMap();
		cave = map->getCave(static_cast<int>(luaL_checkinteger(L, 2) - 1));
	} else {
		IEntity* caveEntity = luaResolveEntity(L, 2, true);
		if (!caveEntity->isCave())
			luaL_error(L, "setTargetCave expects a cave entity or cave index");
		cave = assert_cast<CaveMapTile*, IEntity*>(caveEntity);
	}
	npc->setTargetCave(cave);
	return 0;
}

int luaEntityGetTargetCave (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	if (!entity->isNpcFriendly())
		luaL_error(L, "getTargetCave() is only valid for friendly NPCs");
	NPCFriendly* npc = assert_cast<NPCFriendly*, IEntity*>(entity);
	CaveMapTile* cave = npc->getTargetCave();
	if (cave == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, cave);
	return 1;
}

int luaEntitySetAnimation (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const std::string name = luaL_checkstring(L, 2);
	const Animation* animation = &Animation::getByName(name);
	if (animation->isNone()) {
		const std::string::size_type dash = name.rfind('-');
		if (dash != std::string::npos && dash + 1 < name.size())
			animation = &Animation::getByName(name.substr(dash + 1));
	}
	if (animation->isNone()) {
		Log::error(LOG_GAMEIMPL, "unknown animation '%s'", name.c_str());
		return 0;
	}
	entity->setAnimationType(*animation);
	return 0;
}

int luaEntityGetLightState (lua_State* L)
{
	CaveMapTile* cave = luaGetCaveEntity(L);
	lua_pushboolean(L, cave->getLightState() ? 1 : 0);
	return 1;
}

int luaEntitySetLightState (lua_State* L)
{
	CaveMapTile* cave = luaGetCaveEntity(L);
	cave->setLightState(lua_toboolean(L, 2) != 0);
	return 0;
}

int luaEntitySetNextSpawn (lua_State* L)
{
	CaveMapTile* cave = luaGetCaveEntity(L);
	cave->setNextSpawn(static_cast<uint32_t>(luaL_checkinteger(L, 2)));
	return 0;
}

int luaEntitySetRespawnPossible (lua_State* L)
{
	CaveMapTile* cave = luaGetCaveEntity(L);
	const bool respawn = lua_toboolean(L, 2) != 0;
	const EntityType& type = luaParseEntityType(L, 3, EntityType::NONE);
	cave->setRespawnPossible(respawn, type);
	return 0;
}

int luaEntitySpawnCaveNPC (lua_State* L)
{
	CaveMapTile* cave = luaGetCaveEntity(L);
	const bool spawnPackage = lua_toboolean(L, 2) != 0;
	cave->spawnNPC(spawnPackage);
	INPCCave* npc = cave->getNPC();
	if (npc == nullptr)
		lua_pushnil(L);
	else
		luaPushEntity(L, npc);
	return 1;
}

int luaEntityGetCaveNumber (lua_State* L)
{
	CaveMapTile* cave = luaGetCaveEntity(L);
	lua_pushinteger(L, cave->getCaveNumber());
	return 1;
}

int luaGlobalIsKeyPressed (lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	Map* map = tryGetRuntimeMap(L);
	lua_pushboolean(L, resolveKeyPressed(name, map) ? 1 : 0);
	return 1;
}

void addMapMethod (lua_State* L, const char* name, lua_CFunction fn)
{
	luaL_getmetatable(L, "luaL_Map");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		Log::error(LOG_GAMEIMPL, "Map metatable missing - cannot install %s", name);
		return;
	}
	lua_pushcfunction(L, fn);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

} // anonymous namespace

void MapScript::install (CaveExpressMapContext& ctx)
{
	LUA& lua = ctx.getLua();
	lua_State* L = lua.getState();

	addMapMethod(L, "finish", luaMapFinish);
	addMapMethod(L, "isDone", luaMapIsDone);
	addMapMethod(L, "getTime", luaMapGetTime);
	addMapMethod(L, "getSize", luaMapGetSize);
	addMapMethod(L, "setInputEnabled", luaMapSetInputEnabled);
	addMapMethod(L, "isInputEnabled", luaMapIsInputEnabled);
	addMapMethod(L, "isKeyPressed", luaMapIsKeyPressed);
	addMapMethod(L, "consumeSkip", luaMapConsumeSkip);
	addMapMethod(L, "message", luaMapMessage);
	addMapMethod(L, "getGravity", luaMapGetGravity);
	addMapMethod(L, "calculateVelocity", luaMapCalculateVelocity);
	addMapMethod(L, "getPlayer", luaMapGetPlayer);
	addMapMethod(L, "getEntity", luaMapGetEntity);
	addMapMethod(L, "getCaveCount", luaMapGetCaveCount);
	addMapMethod(L, "getCave", luaMapGetCave);
	addMapMethod(L, "getWaterHeight", luaMapGetWaterHeight);
	addMapMethod(L, "setWaterHeight", luaMapSetWaterHeight);
	addMapMethod(L, "spawnPackage", luaMapSpawnPackage);
	addMapMethod(L, "getPackageCount", luaMapGetPackageCount);
	addMapMethod(L, "getPackage", luaMapGetPackage);
	addMapMethod(L, "getPackages", luaMapGetPackages);
	addMapMethod(L, "getPackageTarget", luaMapGetPackageTarget);
	addMapMethod(L, "getDeliveredPackageCount", luaMapGetDeliveredPackageCount);
	addMapMethod(L, "getCollectedPackageCount", luaMapGetCollectedPackageCount);
	addMapMethod(L, "getPackageDeliveryGoal", luaMapGetPackageDeliveryGoal);
	addMapMethod(L, "spawnPackageNPC", luaMapSpawnPackageNPC);
	addMapMethod(L, "spawnFriendlyNPC", luaMapSpawnFriendlyNPC);
	addMapMethod(L, "spawnNPC", luaMapSpawnNPC);
	addMapMethod(L, "addTileRuntime", luaMapAddTile);
	addMapMethod(L, "removeTileAt", luaMapRemoveTileAt);
	addMapMethod(L, "replaceTile", luaMapReplaceTile);
	addMapMethod(L, "rebuildPlatforms", luaMapRebuildPlatforms);
	addMapMethod(L, "removeEntity", luaMapRemoveEntity);

	luaL_Reg entityFuncs[] = {
			{ "isValid", luaEntityIsValid },
			{ "getId", luaEntityGetId },
			{ "getType", luaEntityGetType },
			{ "getPos", luaEntityGetPos },
			{ "setPos", luaEntitySetPos },
			{ "getVelocity", luaEntityGetVelocity },
			{ "setVelocity", luaEntitySetVelocity },
			{ "applyImpulse", luaEntityApplyImpulse },
			{ "applyForce", luaEntityApplyForce },
			{ "getGravityScale", luaEntityGetGravityScale },
			{ "setGravityScale", luaEntitySetGravityScale },
			{ "getGravity", luaEntityGetGravity },
			{ "getState", luaEntityGetState },
			{ "setState", luaEntitySetState },
			{ "remove", luaEntityRemove },
			{ "isPlayer", luaEntityIsPlayer },
			{ "isNpc", luaEntityIsNpc },
			{ "isCave", luaEntityIsCave },
			{ "accelerate", luaEntityAccelerate },
			{ "resetAcceleration", luaEntityResetAcceleration },
			{ "setMoving", luaEntitySetMoving },
			{ "setIdle", luaEntitySetIdle },
			{ "isIdle", luaEntityIsIdle },
			{ "setDone", luaEntitySetDone },
			{ "returnToCave", luaEntityReturnToCave },
			{ "leavePackage", luaEntityLeavePackage },
			{ "dropPackage", luaEntityDropPackage },
			{ "setInvulnerable", luaEntitySetInvulnerable },
			{ "drop", luaEntityDrop },
			{ "getCollectedPackageCount", luaEntityGetCollectedPackageCount },
			{ "getCollectedPackages", luaEntityGetCollectedPackages },
			{ "isPackage", luaEntityIsPackage },
			{ "isCollected", luaEntityIsCollected },
			{ "isArrived", luaEntityIsArrived },
			{ "isDelivered", luaEntityIsDelivered },
			{ "isDestroyed", luaEntityIsDestroyed },
			{ "isPulling", luaEntityIsPulling },
			{ "getPullingPackage", luaEntityGetPullingPackage },
			{ "setTargetCave", luaEntitySetTargetCave },
			{ "getTargetCave", luaEntityGetTargetCave },
			{ "setAnimation", luaEntitySetAnimation },
			{ "getLightState", luaEntityGetLightState },
			{ "setLightState", luaEntitySetLightState },
			{ "setNextSpawn", luaEntitySetNextSpawn },
			{ "setRespawnPossible", luaEntitySetRespawnPossible },
			{ "spawnCaveNPC", luaEntitySpawnCaveNPC },
			{ "getCaveNumber", luaEntityGetCaveNumber },
			{ nullptr, nullptr }
	};
	lua.reg("Entity", entityFuncs);

	lua_pushcfunction(L, luaGlobalIsKeyPressed);
	lua_setglobal(L, "isKeyPressed");
}

}
