#include "LUAMapContext.h"
#include "common/MapSettings.h"
#include "common/SpriteType.h"
#include "common/Log.h"
#include <memory>
#include <string>

LUAMapContext *LUAMapContext::currentCtx;

LUAMapContext::LUAMapContext (const std::string& name) :
		IMapContext(name), _lua(true), _error(false)
{
	currentCtx = this;
}

LUAMapContext::~LUAMapContext ()
{
	currentCtx = nullptr;
}

void LUAMapContext::initLUABindings (luaL_Reg* additional)
{
	std::vector<luaL_Reg> funcs;
	funcs.push_back(luaL_Reg{ "get", luaGetMapContext });
	funcs.push_back(luaL_Reg{ "addTile", luaAddTile });
	funcs.push_back(luaL_Reg{ "setSetting", luaSetSetting });
	funcs.push_back(luaL_Reg{ "addEmitter", luaAddEmitter });
	funcs.push_back(luaL_Reg{ "addStartPosition", luaAddStartPosition });

	while (additional && additional->name) {
		funcs.push_back(luaL_Reg{ additional->name, additional->func });
		++additional;
	}

	funcs.push_back(luaL_Reg{ nullptr, nullptr });

	_lua.reg("Map", &funcs[0]);
}

void LUAMapContext::addTile (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, EntityAngle angle)
{
	const MapTileDefinition def(x, y, spriteDef, angle);
	_definitions.push_back(def);
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
		Log::info(LOG_COMMON, "could not add tile: %s", tile.c_str());
		ctx->_error = true;
		return 0;
	}

	const EntityAngle angle = luaL_optinteger(l, 5, spriteDefPtr->angle);
	ctx->addTile(spriteDefPtr, x, y, angle);

	return 0;
}

int LUAMapContext::luaAddEmitter (lua_State * l)
{
	LUAMapContext *ctx = _luaGetContext(l, 1);
	const EntityType& type = EntityType::getByName(luaL_checkstring(l, 2));
	const gridCoord x = luaL_checknumber(l, 3);
	const gridCoord y = luaL_checknumber(l, 4);
	const int amount = luaL_optinteger(l, 5, 1);
	const int delay = luaL_optinteger(l, 6, 0);
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
			Log::error(LOG_COMMON, "invalid theme given: %s", value.c_str());
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
		Log::info(LOG_COMMON, "could not load map lua script");
		return false;
	}

	if (!_lua.execute("getName", 1))
		return false;
	_title = _lua.getStringFromStack();
	Log::info(LOG_COMMON, "Load map with title %s", _title.c_str());

	if (!_lua.execute("initMap"))
		return false;

	return !_error;
}

bool LUAMapContext::saveTiles(const FilePtr& file) const {
	bool definitionsAdded = false;
	for (const MapTileDefinition& i : _definitions) {
		file->appendString("\tmap:addTile(\"");
		file->appendString(i.spriteDef->id.c_str());
		file->appendString("\", ");
		file->appendString(string::toString(i.x).c_str());
		file->appendString(", ");
		file->appendString(string::toString(i.y).c_str());
		if (i.angle != 0) {
			file->appendString(", ");
			file->appendString(string::toString(i.angle).c_str());
		}
		file->appendString(")\n");
		definitionsAdded = true;
	}

	if (definitionsAdded || !_emitters.empty())
		file->appendString("\n");

	bool emittersAdded = false;
	for (const EmitterDefinition& i : _emitters) {
		file->appendString("\tmap:addEmitter(\"");
		file->appendString(i.type->name.c_str());
		file->appendString("\", ");
		file->appendString(string::toString(i.x).c_str());
		file->appendString(", ");
		file->appendString(string::toString(i.y).c_str());
		file->appendString(", ");
		file->appendString(string::toString(i.amount).c_str());
		file->appendString(", ");
		file->appendString(string::toString(i.delay).c_str());
		file->appendString(", \"");
		file->appendString(i.settings.c_str());
		file->appendString("\")\n");
		emittersAdded = true;
	}

	return definitionsAdded || emittersAdded;
}

bool LUAMapContext::save() const
{
	const std::string path = FS.getAbsoluteWritePath() + FS.getDataDir() + FS.getMapsDir() + _name + ".lua";
	SDL_RWops *rwops = FS.createRWops(path, "wb");
	FilePtr file(new File(rwops, path));

	file->writeString("function getName()\n");
	file->appendString("\treturn \"");
	file->appendString(_title.c_str());
	file->appendString("\"\n");
	file->appendString("end\n\n");
	file->appendString("function onMapLoaded()\n");
	file->appendString("end\n\n");
	file->appendString("function initMap()\n");
	file->appendString("\t-- get the current map context\n");
	file->appendString("\tlocal map = Map.get()\n");

	if (saveTiles(file)) {
		file->appendString("\n");
	}

	bool settingsAdded = false;
	const IMap::SettingsMap& settings = _settings;
	const auto width = settings.find(msn::WIDTH);
	if (width != settings.end()) {
		file->appendString("\tmap:setSetting(\"");
		file->appendString(msn::WIDTH.c_str());
		file->appendString("\", \"");
		file->appendString(width->second.c_str());
		file->appendString("\")\n");;
		settingsAdded = true;
	}
	const auto height = settings.find(msn::HEIGHT);
	if (height != settings.end()) {
		file->appendString("\tmap:setSetting(\"");
		file->appendString(msn::HEIGHT.c_str());
		file->appendString("\", \"");
		file->appendString(height->second.c_str());
		file->appendString("\")\n");
		settingsAdded = true;
	}
	for (IMap::SettingsMapConstIter i = settings.begin(); i != settings.end(); ++i) {
		if (i->first == msn::WIDTH || i->first == msn::HEIGHT)
			continue;
		file->appendString("\tmap:setSetting(\"");
		file->appendString(i->first.c_str());
		file->appendString("\", \"");
		file->appendString(i->second.c_str());
		file->appendString("\")\n");
		settingsAdded = true;
	}

	if (settingsAdded) {
		file->appendString("\n");
	}

	for (const IMap::StartPosition& pos : _startPositions) {
		file->appendString("\tmap:addStartPosition(\"");
		file->appendString(pos._x.c_str());
		file->appendString("\", \"");
		file->appendString(pos._y.c_str());
		file->appendString("\")\n");
	}

	file->appendString("end\n");

	Log::info(LOG_UI, "wrote %s", path.c_str());

	return true;
}

bool LUAMapContext::isLocationFree (gridCoord x, gridCoord y)
{
	for (std::vector<MapTileDefinition>::const_iterator i = _definitions.begin(); i != _definitions.end(); ++i) {
		const MapTileDefinition& tileDef = *i;
		const SpriteType& type = tileDef.spriteDef->type;
		if (!isSolid(type))
			continue;
		if (tileDef.intersects(x, y))
			return false;
	}
	return true;
}
