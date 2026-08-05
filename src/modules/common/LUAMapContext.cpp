#include "LUAMapContext.h"
#include "common/MapSettings.h"
#include "common/SpriteType.h"
#include "common/Log.h"
#include "common/String.h"
#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

LUAMapContext *LUAMapContext::currentCtx;

namespace {

bool isIdentChar (char c)
{
	return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool matchKeywordAt (const std::string& s, size_t i, const char* keyword)
{
	const size_t n = std::strlen(keyword);
	if (i + n > s.size() || s.compare(i, n, keyword) != 0)
		return false;
	if (i > 0 && isIdentChar(s[i - 1]))
		return false;
	if (i + n < s.size() && isIdentChar(s[i + n]))
		return false;
	return true;
}

/**
 * Advance one character, or skip a full string / comment starting at @p i.
 * Returns the index just past the skipped region.
 */
size_t skipLuaToken (const std::string& s, size_t i)
{
	if (i >= s.size())
		return i;

	// Line comment
	if (s[i] == '-' && i + 1 < s.size() && s[i + 1] == '-') {
		if (i + 3 < s.size() && s[i + 2] == '[') {
			// long comment --[=*[ ... ]=*]
			size_t j = i + 3;
			size_t eqs = 0;
			while (j < s.size() && s[j] == '=') {
				++eqs;
				++j;
			}
			if (j < s.size() && s[j] == '[') {
				++j;
				const std::string closer = "]" + std::string(eqs, '=') + "]";
				const size_t end = s.find(closer, j);
				return end == std::string::npos ? s.size() : end + closer.size();
			}
		}
		while (i < s.size() && s[i] != '\n')
			++i;
		return i;
	}

	// Short string
	if (s[i] == '"' || s[i] == '\'') {
		const char quote = s[i++];
		while (i < s.size() && s[i] != quote) {
			if (s[i] == '\\' && i + 1 < s.size())
				i += 2;
			else
				++i;
		}
		return i < s.size() ? i + 1 : i;
	}

	// Long string [=*[ ... ]=*]
	if (s[i] == '[') {
		size_t j = i + 1;
		size_t eqs = 0;
		while (j < s.size() && s[j] == '=') {
			++eqs;
			++j;
		}
		if (j < s.size() && s[j] == '[') {
			++j;
			const std::string closer = "]" + std::string(eqs, '=') + "]";
			const size_t end = s.find(closer, j);
			return end == std::string::npos ? s.size() : end + closer.size();
		}
	}

	return i + 1;
}

size_t findMatchingFunctionEnd (const std::string& s, size_t functionKeywordPos)
{
	// Start scanning after "function"
	size_t i = functionKeywordPos + 8;
	int depth = 1;
	while (i < s.size()) {
		if (s[i] == '-' || s[i] == '"' || s[i] == '\'' || s[i] == '[') {
			const size_t next = skipLuaToken(s, i);
			if (next != i + 1) {
				i = next;
				continue;
			}
		}

		if (matchKeywordAt(s, i, "function") || matchKeywordAt(s, i, "if") || matchKeywordAt(s, i, "repeat")
				|| matchKeywordAt(s, i, "do")) {
			// for/while open via their trailing "do" — do not also count those keywords.
			++depth;
			while (i < s.size() && isIdentChar(s[i]))
				++i;
			continue;
		}
		if (matchKeywordAt(s, i, "end") || matchKeywordAt(s, i, "until")) {
			--depth;
			const size_t keywordLen = matchKeywordAt(s, i, "until") ? 5 : 3;
			i += keywordLen;
			if (depth == 0)
				return i;
			continue;
		}
		++i;
	}
	return std::string::npos;
}

bool findTopLevelFunctionRange (const std::string& s, const std::string& name, size_t& outStart, size_t& outEnd)
{
	size_t i = 0;
	while (i < s.size()) {
		if (s[i] == '-' || s[i] == '"' || s[i] == '\'' || s[i] == '[') {
			const size_t next = skipLuaToken(s, i);
			if (next != i + 1) {
				i = next;
				continue;
			}
		}

		if (matchKeywordAt(s, i, "function")) {
			size_t j = i + 8;
			while (j < s.size() && std::isspace(static_cast<unsigned char>(s[j])))
				++j;
			if (s.compare(j, name.size(), name) == 0 && (j + name.size() >= s.size() || !isIdentChar(s[j + name.size()]))) {
				size_t k = j + name.size();
				while (k < s.size() && std::isspace(static_cast<unsigned char>(s[k])))
					++k;
				if (k < s.size() && s[k] == '(') {
					const size_t end = findMatchingFunctionEnd(s, i);
					if (end == std::string::npos)
						return false;
					outStart = i;
					outEnd = end;
					return true;
				}
			}
		}
		++i;
	}
	return false;
}

void eraseTopLevelFunction (std::string& s, const std::string& name)
{
	size_t start = 0;
	size_t end = 0;
	if (!findTopLevelFunctionRange(s, name, start, end))
		return;

	// Also drop blank lines immediately surrounding the removed function.
	while (start > 0 && (s[start - 1] == ' ' || s[start - 1] == '\t'))
		--start;
	if (start > 0 && s[start - 1] == '\n')
		--start;
	while (end < s.size() && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r'))
		++end;
	if (end < s.size() && s[end] == '\n')
		++end;

	s.erase(start, end - start);
}

std::string collapseExcessBlankLines (const std::string& input)
{
	std::string out;
	out.reserve(input.size());
	int newlines = 0;
	for (char c : input) {
		if (c == '\n') {
			if (++newlines <= 2)
				out.push_back(c);
		} else {
			newlines = 0;
			out.push_back(c);
		}
	}
	return string::trim(out);
}

}

LUAMapContext::LUAMapContext (const std::string& name) :
		IMapContext(name), _lua(true), _error(false), _hasOnMapLoaded(false), _hasOnUpdate(false)
{
	currentCtx = this;
}

LUAMapContext::~LUAMapContext ()
{
	if (currentCtx == this)
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
	const std::string &name = luaL_checkstring(l, 2);
	const EntityType& type = EntityType::getByName(name);
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
	if (!_hasOnMapLoaded)
		return;
	currentCtx = this;
	if (!_lua.execute("onMapLoaded")) {
		_hasOnMapLoaded = false;
	}
}

void LUAMapContext::onUpdate (uint32_t deltaTime)
{
	if (!_hasOnUpdate)
		return;
	currentCtx = this;
	if (!_lua.execute("onUpdate", static_cast<double>(deltaTime))) {
		// Avoid spamming the log every frame if the script errors.
		_hasOnUpdate = false;
	}
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

void LUAMapContext::capturePreservedLogic (const std::string& source)
{
	std::string logic = source;
	eraseTopLevelFunction(logic, "getName");
	eraseTopLevelFunction(logic, "initMap");
	_preservedLogic = collapseExcessBlankLines(logic);
}

bool LUAMapContext::load (bool skipErrors)
{
	resetTiles();
	_preservedLogic.clear();

	const std::string mapFile = FS.getMapsDir() + _name + ".lua";
	const FilePtr file = FS.getFile(mapFile);
	if (!file->exists()) {
		Log::info(LOG_COMMON, "could not load map lua script");
		return false;
	}

	char* buffer = nullptr;
	const int fileLen = file->read((void**)&buffer);
	std::unique_ptr<char[]> holder(buffer);
	if (buffer == nullptr || fileLen <= 0) {
		Log::info(LOG_COMMON, "could not read map lua script");
		return false;
	}

	const std::string source(buffer, fileLen);
	capturePreservedLogic(source);

	if (!_lua.loadBuffer(source, _name.c_str())) {
		Log::info(LOG_COMMON, "could not load map lua script");
		return false;
	}

	if (!_lua.execute("getName", 1))
		return false;
	_title = _lua.getStringFromStack();
	Log::info(LOG_COMMON, "Load map with title %s", _title.c_str());

	if (!_lua.execute("initMap"))
		return false;

	_hasOnMapLoaded = _lua.hasFunction("onMapLoaded");
	_hasOnUpdate = _lua.hasFunction("onUpdate");

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

	if (!_preservedLogic.empty()) {
		file->appendString(_preservedLogic.c_str());
		if (_preservedLogic.back() != '\n')
			file->appendString("\n");
		file->appendString("\n");
	} else {
		// New / logic-free maps keep a trivial onMapLoaded hook for editor compatibility.
		file->appendString("function onMapLoaded()\n");
		file->appendString("end\n\n");
	}

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

	if (file->length() <= 0L) {
		FS.deleteFile(path);
		return false;
	}

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
