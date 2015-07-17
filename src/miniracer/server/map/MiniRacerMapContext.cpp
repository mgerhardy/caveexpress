#include "MiniRacerMapContext.h"
#include "common/MapSettings.h"
#include "miniracer/shared/MiniRacerSpriteType.h"
#include <memory>
#include "common/FileSystem.h"
#include "common/Log.h"
#include <string>

namespace miniracer {

MiniRacerMapContext *MiniRacerMapContext::currentCtx;

MiniRacerMapContext::MiniRacerMapContext (const std::string& name) :
		IMapContext(name), _error(false)
{
	currentCtx = this;
	luaL_Reg funcs[] = {
		{ "get", luaGetMapContext },
		{ "addTile", luaAddTile },
		{ "setSetting", luaSetSetting },
		{ "addEmitter", luaAddEmitter },
		{ "addStartPosition", luaAddStartPosition },
		{ nullptr, nullptr }
	};
	_lua.reg("Map", funcs);
}

MiniRacerMapContext::~MiniRacerMapContext ()
{
	currentCtx = nullptr;
}

void MiniRacerMapContext::addTile (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, EntityAngle angle)
{
	const MapTileDefinition def(x, y, spriteDef, angle);
	_definitions.push_back(def);
}

void MiniRacerMapContext::addEmitter (const EntityType& type, gridCoord x, gridCoord y, int amount, int delay, const std::string& settings)
{
	const EmitterDefinition def(x, y, type, amount, delay, settings);
	_emitters.push_back(def);
}

int MiniRacerMapContext::luaGetMapContext (lua_State * l)
{
	MiniRacerMapContext ** udata = LUA::newUserdata<MiniRacerMapContext>(l, "Map");
	*udata = currentCtx;
	return 1;
}

MiniRacerMapContext* MiniRacerMapContext::_luaGetContext (lua_State * l, int n)
{
	return LUA::getUserData<MiniRacerMapContext>(l, n, "Map");
}

int MiniRacerMapContext::luaAddTile (lua_State * l)
{
	MiniRacerMapContext *ctx = _luaGetContext(l, 1);
	const std::string tile = luaL_checkstring(l, 2);
	const gridCoord x = luaL_checknumber(l, 3);
	const gridCoord y = luaL_checknumber(l, 4);

	SpriteDefPtr spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(tile);
	if (!spriteDefPtr) {
		Log::info(LOG_SERVER, "could not add tile: %s", tile.c_str());
		ctx->_error = true;
		return 0;
	}

	const EntityAngle angle = luaL_optinteger(l, 5, spriteDefPtr->angle);
	ctx->addTile(spriteDefPtr, x, y, angle);

	return 0;
}

int MiniRacerMapContext::luaAddEmitter (lua_State * l)
{
	MiniRacerMapContext *ctx = _luaGetContext(l, 1);
	const EntityType& type = EntityType::getByName(luaL_checkstring(l, 2));
	const gridCoord x = luaL_checknumber(l, 3);
	const gridCoord y = luaL_checknumber(l, 4);
	const int amount = luaL_optinteger(l, 5, 1);
	const int delay = luaL_optinteger(l, 6, 0);
	const std::string settings = luaL_optstring(l, 7, "");
	ctx->addEmitter(type, x, y, amount, delay, settings);

	return 0;
}

void MiniRacerMapContext::onMapLoaded ()
{
	_lua.execute("onMapLoaded");
}

int MiniRacerMapContext::luaAddStartPosition (lua_State * l) {
	MiniRacerMapContext *ctx = _luaGetContext(l, 1);
	const std::string x = luaL_checkstring(l, 2);
	const std::string y = luaL_checkstring(l, 3);

	const IMap::StartPosition p{x, y};
	ctx->_startPositions.push_back(p);

	return 0;
}

int MiniRacerMapContext::luaSetSetting (lua_State * l)
{
	MiniRacerMapContext *ctx = _luaGetContext(l, 1);
	const std::string key = luaL_checkstring(l, 2);
	const std::string value = luaL_checkstring(l, 3);

	if (key == msn::THEME) {
		ctx->_theme = &ThemeType::getByName(value);
		if (ctx->_theme->isNone()) {
			Log::error(LOG_SERVER, "invalid theme given: %s", value.c_str());
			ctx->_theme = &ThemeTypes::ROCK;
		}
	}

	ctx->_settings[key] = value;

	return 0;
}

bool MiniRacerMapContext::load (bool skipErrors)
{
	resetTiles();
	if (!_lua.load(FS.getMapsDir() + _name + ".lua")) {
		Log::info(LOG_SERVER, "could not load map lua script");
		return false;
	}

	if (!_lua.execute("getName", 1))
		return false;
	_title = _lua.getStringFromStack();
	Log::info(LOG_SERVER, "Load map with title %s", _title.c_str());

	if (!_lua.execute("initMap"))
		return false;

	return !_error;
}

bool MiniRacerMapContext::isLocationFree (gridCoord x, gridCoord y)
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

}
