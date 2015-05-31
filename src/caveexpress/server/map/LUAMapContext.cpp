#include "LUAMapContext.h"
#include "common/MapSettings.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "common/Pointers.h"
#include "common/Logger.h"
#include <string>

LUAMapContext *LUAMapContext::currentCtx;

LUAMapContext::LUAMapContext (const std::string& name) :
		IMapContext(name), _error(false)
{
	currentCtx = this;
	luaL_Reg funcs[] = {
		{ "get", luaGetMapContext },
		{ "addTile", luaAddTile },
		{ "addCave", luaAddCave },
		{ "setSetting", luaSetSetting },
		{ "addEmitter", luaAddEmitter },
		{ "addStartPosition", luaAddStartPosition },
		{ nullptr, nullptr }
	};
	_lua.reg("Map", funcs);
}

LUAMapContext::~LUAMapContext ()
{
	currentCtx = nullptr;
}

void LUAMapContext::addTile (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, EntityAngle angle)
{
	const MapTileDefinition def(x, y, spriteDef, angle);
	_definitions.push_back(def);
}

void LUAMapContext::addCave (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, const EntityType& entityType, int delay)
{
	const CaveTileDefinition def(x, y, spriteDef, entityType, delay);
	_caveDefinitions.push_back(def);
}

void LUAMapContext::addEmitter (const EntityType& type, gridCoord x, gridCoord y, int amount, int delay, const std::string& settings)
{
	const EmitterDefinition def(x, y, type, amount, delay, settings);
	_emitters.push_back(def);
}

int LUAMapContext::luaGetMapContext (lua_State * l)
{
	LUAMapContext ** udata = LUA::newUserdata<LUAMapContext>(l, "Map");
	*udata = currentCtx;
	return 1;
}

LUAMapContext* LUAMapContext::_luaGetContext (lua_State * l, int n)
{
	return LUA::getUserData<LUAMapContext>(l, n, "Map");
}

int LUAMapContext::luaAddTile (lua_State * l)
{
	LUAMapContext *ctx = _luaGetContext(l, 1);
	const std::string tile = luaL_checkstring(l, 2);
	const gridCoord x = luaL_checknumber(l, 3);
	const gridCoord y = luaL_checknumber(l, 4);

	SpriteDefPtr spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(tile);
	if (!spriteDefPtr) {
		info(LOG_SERVER, "could not add tile: " + tile);
		ctx->_error = true;
		return 0;
	}

	const EntityAngle angle = luaL_optinteger(l, 5, spriteDefPtr->angle);
	ctx->addTile(spriteDefPtr, x, y, angle);

	return 0;
}

int LUAMapContext::luaAddCave (lua_State * l)
{
	LUAMapContext *ctx = _luaGetContext(l, 1);
	const std::string caveTile = luaL_checkstring(l, 2);
	const gridCoord x = luaL_checknumber(l, 3);
	const gridCoord y = luaL_checknumber(l, 4);
	const EntityType& entityType = EntityType::getByName(luaL_optstring(l, 5, "none"));
	const int delay = luaL_optinteger(l, 6, 5000);

	SpriteDefPtr spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(caveTile);
	if (!spriteDefPtr) {
		info(LOG_SERVER, "could not add cave: " + caveTile);
		ctx->_error = true;
		return 0;
	}

	ctx->addCave(spriteDefPtr, x, y, entityType, delay);

	return 0;
}

int LUAMapContext::luaAddEmitter (lua_State * l)
{
	LUAMapContext *ctx = _luaGetContext(l, 1);
	const EntityType& type = EntityType::getByName(luaL_checkstring(l, 2));
	const gridCoord x = luaL_checknumber(l, 3);
	const gridCoord y = luaL_checknumber(l, 4);
	const int amount = luaL_optint(l, 5, 1);
	const int delay = luaL_optint(l, 6, 0);
	const std::string settings = luaL_optstring(l, 7, "");
	ctx->addEmitter(type, x, y, amount, delay, settings);

	return 0;
}

void LUAMapContext::onMapLoaded ()
{
	_lua.execute("onMapLoaded");
}

int LUAMapContext::luaAddStartPosition (lua_State * l) {
	LUAMapContext *ctx = _luaGetContext(l, 1);
	const std::string x = luaL_checkstring(l, 2);
	const std::string y = luaL_checkstring(l, 3);

	const IMap::StartPosition p{x, y};
	ctx->_startPositions.push_back(p);

	return 0;
}

int LUAMapContext::luaSetSetting (lua_State * l)
{
	LUAMapContext *ctx = _luaGetContext(l, 1);
	const std::string key = luaL_checkstring(l, 2);
	const std::string value = luaL_checkstring(l, 3);

	if (key == msn::THEME) {
		ctx->_theme = &ThemeType::getByName(value);
		if (ctx->_theme->isNone()) {
			error(LOG_SERVER, "invalid theme given: " + value);
			ctx->_theme = &ThemeTypes::ROCK;
		}
	}

	ctx->_settings[key] = value;

	return 0;
}

bool LUAMapContext::load (bool skipErrors)
{
	resetTiles();
	if (!_lua.load(FS.getMapsDir() + _name + ".lua")) {
		info(LOG_SERVER, "could not load map lua script");
		return false;
	}

	if (!_lua.execute("getName", 1))
		return false;
	_title = _lua.getStringFromStack();

	if (!_lua.execute("initMap"))
		return false;

	return !_error;
}

bool LUAMapContext::isLocationFree (gridCoord x, gridCoord y)
{
	for (std::vector<MapTileDefinition>::const_iterator i = _definitions.begin(); i != _definitions.end(); ++i) {
		const MapTileDefinition& tileDef = *i;
		const SpriteType& type = tileDef.spriteDef->type;
		if (!SpriteTypes::isSolid(type))
			continue;
		if (tileDef.intersects(x, y))
			return false;
	}
	return true;
}
