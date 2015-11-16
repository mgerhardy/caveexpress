#include "LUA.h"
#include "common/FileSystem.h"
#include "common/Config.h"
#include "common/System.h"

LUA::LUA (bool debug)
{
	init(debug);
}

LUA::~LUA ()
{
	close();
}

void LUA::close ()
{
	lua_close(_state);
}

void LUA::init (bool debug)
{
	_state = luaL_newstate();
	luaL_openlibs(_state);

	lua_register(_state, "isAndroid", isAndroid);
	lua_register(_state, "isWindows", isWindows);
	lua_register(_state, "isMacOSX", isMacOSX);
	lua_register(_state, "isIOS", isIOS);
	lua_register(_state, "isLinux", isLinux);
	lua_register(_state, "isOUYA", isOUYA);
	lua_register(_state, "isHTML5", isHTML5);
	lua_register(_state, "isDebug", isDebug);
	lua_register(_state, "isHD", isHD);
	lua_register(_state, "isNaCl", isNaCl);
	lua_register(_state, "isTouch", isTouch);

	if (debug) {
		const int mask = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
		lua_sethook(_state, debugHook, mask, 0);
	}

	lua_atpanic(_state, panicHook);
}

void LUA::reg (const std::string& prefix, luaL_Reg* funcs)
{
	const std::string metaTableName = "luaL_" + prefix;
	luaL_newmetatable(_state, metaTableName.c_str());
	luaL_setfuncs(_state, funcs, 0);
	lua_pushvalue(_state, -1);
	lua_setfield(_state, -1, "__index");
	lua_setglobal(_state, prefix.c_str());
}

bool LUA::load (const std::string &file)
{
	FilePtr filePtr = FS.getFile(file);
	if (!filePtr->exists()) {
		Log::error(LOG_COMMON, "lua file '%s' does not exist", filePtr->getName().c_str());
		return false;
	}

	char *buffer;
	const int fileLen = filePtr->read((void **) &buffer);
	std::unique_ptr<char[]> p(buffer);
	if (!buffer || fileLen <= 0) {
		Log::error(LOG_COMMON, "failed to read lua file %s", filePtr->getName().c_str());
		return false;
	}

	return loadBuffer(std::string(buffer, fileLen), file.c_str());
}

bool LUA::loadBuffer (const std::string& buffer, const char *ctx)
{
	if (luaL_loadbufferx(_state, buffer.c_str(), buffer.size(), ctx, nullptr) || lua_pcall(_state, 0, 0, 0)) {
		Log::error(LOG_COMMON, "%s: %s", ctx, lua_tostring(_state, -1));
		pop();
		return false;
	}

	return true;
}

bool LUA::getValueBoolFromTable (const char * key, bool defaultValue)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return defaultValue;
	}
	LUA_checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		pop();
		return defaultValue;
	}

	const bool rtn = lua_toboolean(_state, -1);
	pop();
	return rtn;
}

char LUA::getValueCharFromTable (const char * key, const char defaultValue)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return defaultValue;
	}
	LUA_checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		pop();
		return defaultValue;
	}

	const char *str = luaL_checkstring(_state, -1);
	pop();
	SDL_assert_always(SDL_strlen(str) == 1);
	return str[0];
}

std::string LUA::getValueStringFromTable (const char * key, const std::string& defaultValue)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return defaultValue;
	}
	LUA_checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		pop();
		return defaultValue;
	}

	const std::string rtn = luaL_checkstring(_state, -1);
	pop();
	return rtn;
}

float LUA::getValueFloatFromTable (const char * key, float defaultValue)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return defaultValue;
	}
	LUA_checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		pop();
		return defaultValue;
	}

	const float rtn = static_cast<float>(luaL_checknumber(_state,-1));
	pop();
	return rtn;
}

int LUA::getValueIntegerFromTable (const char * key, int defaultValue)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return defaultValue;
	}
	LUA_checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		pop();
		return defaultValue;
	}

	const int rtn = luaL_checkinteger(_state, -1);
	pop();
	return rtn;
}

/**
 * @param[in] function function to be called
 */
bool LUA::execute (const std::string &function, int returnValues)
{
	if (!getGlobal(function))
		return false;
	const int ret = lua_pcall(_state, 0, returnValues, 0);
	if (ret != 0) {
		const char * s = luaL_checkstring(_state, -1);
		if (s == nullptr)
			Log::error(LOG_COMMON, "unrecognized Lua error");
		else
			Log::error(LOG_COMMON, "%s", s);
		return false;
	}

	return true;
}

std::string LUA::getLuaValue (int stackIndex)
{
	const int t = lua_type(_state, stackIndex);
	switch (t) {
	case LUA_TNUMBER:
	case LUA_TSTRING:
		lua_pushstring(_state, lua_tostring(_state, stackIndex));
		break;
	case LUA_TBOOLEAN:
		lua_pushstring(_state, (lua_toboolean(_state, stackIndex) ? "true" : "false"));
		break;
	case LUA_TNIL:
		lua_pushliteral(_state, "nil");
		break;
	default:
		lua_pushfstring(_state, "%s: %p", luaL_typename(_state, stackIndex), lua_topointer(_state, stackIndex));
		break;
	}

	std::string result(lua_tostring(_state, -1));
	pop();
	return result;
}

void LUA::tableDump()
{
	lua_pushnil(_state);
	while (getNextKeyValue()) {
		const std::string& key = getLuaValue(-2);
		const std::string& value = getLuaValue(-1);
		Log::info(LOG_COMMON, "%s : %s", key.c_str(), value.c_str());
		pop();
	}
}

std::string LUA::getStackDump ()
{
	LUA_checkStack();
	const int top = lua_gettop(_state);
	std::string sd = string::format("stack elements: %i\n", top);
	for (int i = 1; i <= top; i++) { /* repeat for each level */
		sd += getLuaValue(i);
		sd += "\n";
	}
	return sd;
}

void LUA::stackDump ()
{
	Log::info(LOG_COMMON, "%s", getStackDump().c_str());
}

std::string LUA::getStringFromStack ()
{
	const char* id = luaL_checkstring(_state, -1);
	pop();
	if (id == nullptr)
		return "";
	return id;
}

std::string LUA::getString (const std::string& expr, const std::string& defaultValue)
{
	LUA_checkStack();
	std::string r = defaultValue;
	/* Assign the Lua expression to a Lua global variable. */
	const std::string buf("return " + expr);
	Log::debug(LOG_COMMON, "eval: '%s'", buf.c_str());
	if (!luaL_dostring(_state, buf.c_str())) {
		const char *str = lua_tostring(_state, -1);
		if (str != nullptr)
			r = str;
		/* remove lua_getglobal value */
		pop();
	}
	return r;
}

void LUA::getKeyValueMap (std::map<std::string, std::string>& map, const std::string& key)
{
	LUA_checkStack();
	if (!getGlobalKeyValue(key)) {
		return;
	}
	while (getNextKeyValue()) {
		const std::string& _key = getLuaValue(-2);
		const std::string& _value = getLuaValue(-1);
		map[_key] = _value;
		pop();
	}

	pop();
}

int LUA::getIntValue (const std::string& path, int defaultValue)
{
	return string::toInt(getString(path), defaultValue);
}

float LUA::getFloatValue (const std::string& path, float defaultValue)
{
	return string::toFloat(getString(path), defaultValue);
}

bool LUA::getBoolValue (const std::string& path)
{
	return string::toBool(getString(path));
}

bool LUA::getGlobalKeyValue (const std::string& name)
{
	if (!getGlobal(name)) {
		return false;
	}
	lua_pushnil(_state);
	return true;
}

int LUA::getTable (const std::string& name)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return 0;
	}
	lua_getfield(_state, -1, name.c_str());
	if (lua_isnil(_state,-1)) {
		pop();
		return -1;
	}
	return lua_rawlen(_state, -1);
}

std::string LUA::getTableString (int i)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return "";
	}
	LUA_checkStack();
	lua_rawgeti(_state, -1, i);
	const std::string str = luaL_checkstring(_state, -1);
	pop();
	return str;
}

bool LUA::getTableBool (int i)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return false;
	}
	LUA_checkStack();
	lua_rawgeti(_state, -1, i);
	const bool val = lua_toboolean(_state, -1);
	pop();
	return val;
}

int LUA::getTableInteger (int i)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return 0;
	}
	lua_rawgeti(_state, -1, i);
	const int val = luaL_checkinteger(_state, -1);
	pop();
	return val;
}

float LUA::getTableFloat (int i)
{
	if (!lua_istable(_state, -1)) {
		Log::error(LOG_COMMON, "expected a lua table at the top of the stack");
		stackDump();
		return 0.0f;
	}
	lua_rawgeti(_state, -1, i);
	const float val = luaL_checknumber(_state, -1);
	pop();
	return val;
}

std::string LUA::getKey ()
{
	return luaL_checkstring(_state, -2);
}

void LUA::pop (int amount)
{
	lua_pop(_state, amount);
}

int LUA::stackCount ()
{
	return lua_gettop(_state);
}

bool LUA::getNextKeyValue ()
{
	return lua_next(_state, -2) != 0;
}

bool LUA::getGlobal (const std::string& name)
{
	lua_getglobal(_state, name.c_str());
	if (lua_isnil(_state, -1)) {
		Log::error(LOG_COMMON, "Could not find %s lua global", name.c_str());
		return false;
	}
	return true;
}

void LUA::debugHook (lua_State *L, lua_Debug *ar)
{
	if (!lua_getinfo(L, "Sn", ar))
		return;

	Log::debug(LOG_COMMON, "%s %s: %s %d", ar->namewhat, ar->name, ar->short_src, ar->currentline);
}

int LUA::panicHook (lua_State *L)
{
	Log::error(LOG_COMMON, "Lua panic. Error message: %s", lua_isnil(L, -1) ? "" : lua_tostring(L, -1));
	return 0;
}

int LUA::isAndroid (lua_State *L)
{
#if defined(__ANDROID__)
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isWindows (lua_State *L)
{
#if defined(__WIN32__)
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isMacOSX (lua_State *L)
{
#if defined(__MACOSX__)
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isIOS (lua_State *L)
{
#if defined(__IPHONEOS__)
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isLinux (lua_State *L)
{
#if defined(__LINUX__)
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isHTML5 (lua_State *L)
{
#ifdef EMSCRIPTEN
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isDebug (lua_State *L)
{
#ifdef DEBUG
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isTouch (lua_State *L)
{
#if defined(__ANDROID__)
	Android& system = static_cast<Android&>(getSystem());
	lua_pushboolean(L, !system.isOUYA());
#else
#if defined(__IPHONEOS__)
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
#endif
	return 1;
}

int LUA::isHD (lua_State *L)
{
#ifdef HD_VERSION
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isOUYA (lua_State *L)
{
#if defined(__ANDROID__)
	Android& system = static_cast<Android&>(getSystem());
	lua_pushboolean(L, system.isOUYA());
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}

int LUA::isNaCl (lua_State *L)
{
#if defined(__NACL__)
	lua_pushboolean(L, true);
#else
	lua_pushboolean(L, false);
#endif
	return 1;
}
