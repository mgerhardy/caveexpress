#include "MapScript.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/server/entities/IEntity.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/MapTile.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/npcs/NPC.h"
#include "caveexpress/server/entities/npcs/NPCFriendly.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/server/entities/npcs/NPCFlying.h"
#include "caveexpress/server/entities/npcs/NPCFish.h"
#include "caveexpress/server/entities/npcs/NPCBlowing.h"
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

IEntity** luaPushEntity (lua_State* L, IEntity* entity)
{
	IEntity** udata = LUA::newUserdata<IEntity>(L, "Entity");
	*udata = entity;
	return udata;
}

IEntity* luaGetEntity (lua_State* L, int n = 1)
{
	IEntity* entity = LUA::getUserData<IEntity>(L, n, "Entity");
	if (entity == nullptr)
		luaL_error(L, "invalid entity");
	return entity;
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

bool resolveKeyPressed (const std::string& name)
{
	if (isPhysicalKeyPressed(name))
		return true;

	// Convenience aliases used in map scripts
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
	if (!SDL_strcasecmp(name.c_str(), "skip") || !SDL_strcasecmp(name.c_str(), "any")) {
		// any of the common gameplay keys
		return resolveKeyPressed("drop") || resolveKeyPressed("left") || resolveKeyPressed("right")
				|| resolveKeyPressed("up") || resolveKeyPressed("down") || isPhysicalKeyPressed("Escape")
				|| isPhysicalKeyPressed("Space") || isPhysicalKeyPressed("Return");
	}

	return isCommandBoundKeyPressed(name);
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
	luaRequireMap(L); // ensure we are in a running map
	const char* name = luaL_checkstring(L, 2);
	lua_pushboolean(L, resolveKeyPressed(name) ? 1 : 0);
	return 1;
}

int luaMapMessage (lua_State* L)
{
	Map* map = luaRequireMap(L);
	const char* text = luaL_checkstring(L, 2);
	const Map::PlayerList& players = map->getPlayers();
	for (Player* player : players)
		map->sendMessage(player->getClientId(), text);
	// also try waiting players (before start)
	return 0;
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

int luaMapRemoveEntity (lua_State* L)
{
	Map* map = luaRequireMap(L);
	IEntity* entity = nullptr;
	if (lua_isuserdata(L, 2)) {
		entity = luaGetEntity(L, 2);
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
	assert_cast<NPCPackage*, IEntity*>(entity)->leavePackage();
	return 0;
}

int luaEntitySetAnimation (lua_State* L)
{
	IEntity* entity = luaGetEntity(L);
	const std::string name = luaL_checkstring(L, 2);
	const Animation& animation = Animation::getByName(name);
	if (animation.isNone())
		luaL_error(L, "unknown animation '%s'", name.c_str());
	entity->setAnimationType(animation);
	return 0;
}

int luaGlobalIsKeyPressed (lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	lua_pushboolean(L, resolveKeyPressed(name) ? 1 : 0);
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
	addMapMethod(L, "message", luaMapMessage);
	addMapMethod(L, "getPlayer", luaMapGetPlayer);
	addMapMethod(L, "getEntity", luaMapGetEntity);
	addMapMethod(L, "getCaveCount", luaMapGetCaveCount);
	addMapMethod(L, "getCave", luaMapGetCave);
	addMapMethod(L, "getWaterHeight", luaMapGetWaterHeight);
	addMapMethod(L, "spawnPackage", luaMapSpawnPackage);
	addMapMethod(L, "spawnPackageNPC", luaMapSpawnPackageNPC);
	addMapMethod(L, "spawnFriendlyNPC", luaMapSpawnFriendlyNPC);
	addMapMethod(L, "spawnNPC", luaMapSpawnNPC);
	addMapMethod(L, "addTileRuntime", luaMapAddTile);
	addMapMethod(L, "removeEntity", luaMapRemoveEntity);

	luaL_Reg entityFuncs[] = {
			{ "getId", luaEntityGetId },
			{ "getType", luaEntityGetType },
			{ "getPos", luaEntityGetPos },
			{ "setPos", luaEntitySetPos },
			{ "getState", luaEntityGetState },
			{ "setState", luaEntitySetState },
			{ "remove", luaEntityRemove },
			{ "isPlayer", luaEntityIsPlayer },
			{ "isNpc", luaEntityIsNpc },
			{ "isCave", luaEntityIsCave },
			{ "accelerate", luaEntityAccelerate },
			{ "setMoving", luaEntitySetMoving },
			{ "setIdle", luaEntitySetIdle },
			{ "setDone", luaEntitySetDone },
			{ "returnToCave", luaEntityReturnToCave },
			{ "leavePackage", luaEntityLeavePackage },
			{ "setAnimation", luaEntitySetAnimation },
			{ nullptr, nullptr }
	};
	lua.reg("Entity", entityFuncs);

	lua_pushcfunction(L, luaGlobalIsKeyPressed);
	lua_setglobal(L, "isKeyPressed");
}

}
