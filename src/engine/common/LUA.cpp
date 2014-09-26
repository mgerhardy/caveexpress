#include "LUA.h"
#include "engine/common/FileSystem.h"
#include "engine/common/Config.h"
#include "engine/common/System.h"
#include <assert.h>

class StackChecker {
private:
	lua_State *_state;
	const int _startStackDepth;
public:
	StackChecker (lua_State *state) :
			_state(state), _startStackDepth(lua_gettop(_state))
	{
	}
	~StackChecker ()
	{
		assert(_startStackDepth == lua_gettop(_state));
	}
};

#ifdef DEBUG
#define checkStack() StackChecker(this->_state)
#else
#define checkStack() do {} while(0)
#endif

LUA::LUA (bool debug)
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
}

LUA::~LUA ()
{
	lua_close(_state);
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
		error(LOG_CONFIG, "lua file " + filePtr->getName() + " does not exist");
		return false;
	}

	char *buffer;
	const int fileLen = filePtr->read((void **) &buffer);
	ScopedArrayPtr<char> p(buffer);
	if (!buffer || fileLen <= 0) {
		error(LOG_CONFIG, "failed to read lua file " + filePtr->getName());
		return false;
	}

	if (luaL_loadbufferx(_state, buffer, fileLen, file.c_str(), nullptr) || lua_pcall(_state, 0, 0, 0)) {
		error(LOG_LUA, file + ": " + lua_tostring(_state, -1));
		pop(1);
		return false;
	}

	return true;
}

bool LUA::getValueBoolFromTable (const char * key, bool defaultValue)
{
	checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		lua_pop(_state, 1);
		return defaultValue;
	}

	const bool rtn = lua_toboolean(_state, -1);
	lua_pop(_state, 1);
	return rtn;
}

String LUA::getValueStringFromTable (const char * key, const String& defaultValue)
{
	checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		lua_pop(_state, 1);
		return defaultValue;
	}

	const String rtn = lua_tostring(_state,-1);
	lua_pop(_state, 1);
	return rtn;
}

float LUA::getValueFloatFromTable (const char * key, float defaultValue)
{
	checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		lua_pop(_state, 1);
		return defaultValue;
	}

	const float rtn = static_cast<float>(lua_tonumber(_state,-1));
	lua_pop(_state, 1);
	return rtn;
}

int LUA::getValueIntegerFromTable (const char * key, int defaultValue)
{
	checkStack();
	lua_getfield(_state, -1, key);
	if (lua_isnil(_state, -1)) {
		lua_pop(_state, 1);
		return defaultValue;
	}

	const int rtn = lua_tointeger(_state, -1);
	lua_pop(_state, 1);
	return rtn;
}

/**
 * @param[in] function function to be called
 */
bool LUA::execute (const String &function, int returnValues)
{
	lua_getglobal(_state, function.c_str());
	const int ret = lua_pcall(_state, 0, returnValues, 0);
	if (ret != 0) {
		return false;
	}

	return true;
}

void LUA::stackDump ()
{
	checkStack();
	const int top = lua_gettop(_state);
	for (int i = 1; i <= top; i++) { /* repeat for each level */
		const int t = lua_type(_state, i);
		printf("%i: ", i);
		switch (t) {
		case LUA_TSTRING:
			printf("'%s'", lua_tostring(_state, i));
			break;

		case LUA_TBOOLEAN:
			printf(lua_toboolean(_state, i) ? "true" : "false");
			break;

		case LUA_TNUMBER:
			printf("%g", lua_tonumber(_state, i));
			break;

		default:
			printf("%s", lua_typename(_state, t));
			break;

		}
		printf("\n");
	}
}

std::string LUA::getStringFromStack ()
{
	const char* id = lua_tostring(_state, -1);
	pop();
	if (id == nullptr)
		return "";
	return id;
}

String LUA::getString (const std::string& expr, const std::string& defaultValue)
{
	checkStack();
	const char* r = defaultValue.c_str();
	/* Assign the Lua expression to a Lua global variable. */
	const std::string buf("evalExpr=" + expr);
	if (!luaL_dostring(_state, buf.c_str())) {
		/* Get the value of the global variable */
		lua_getglobal(_state, "evalExpr");
		if (lua_isstring(_state, -1))
			r = lua_tostring(_state, -1);
		else if (lua_isboolean(_state, -1))
			r = lua_toboolean(_state, -1) ? "true" : "false";
		/* remove lua_getglobal value */
		lua_pop(_state, 1);
	}
	return r;
}

void LUA::getKeyValueMap (std::map<std::string, std::string>& map, const char *key)
{
	checkStack();
	lua_getglobal(_state, key);
	lua_pushnil(_state);

	while (lua_next(_state, -2) != 0) {
		const char *_key = lua_tostring(_state, -2);
		assert(_key);
		std::string _value;
		if (lua_isstring(_state, -1)) {
			_value = lua_tostring(_state, -1);
		} else if (lua_isnumber(_state, -1)) {
			_value = string::toString(lua_tonumber(_state, -1));
		} else if (lua_isboolean(_state, -1)) {
			_value = lua_toboolean(_state, -1) ? "true" : "false";
		}
		map[_key] = _value;
		lua_pop(_state, 1);
	}

	lua_pop(_state, 1);
}

int LUA::getIntValue (const std::string& path, int defaultValue)
{
	return getString(path).toInt(defaultValue);
}

float LUA::getFloatValue (const std::string& path, float defaultValue)
{
	return getString(path).toFloat(defaultValue);
}

bool LUA::getBoolValue (const std::string& path)
{
	return getString(path).toBool();
}

void LUA::getGlobalKeyValue (const std::string& name)
{
	lua_getglobal(_state, name.c_str());
	lua_pushnil(_state);
}

int LUA::getTable (const std::string& name)
{
	lua_getfield(_state, -1, name.c_str());
	return lua_rawlen(_state, -1);
}

std::string LUA::getTableString (int i)
{
	checkStack();
	lua_rawgeti(_state, -1, i);
	const std::string str = lua_tostring(_state, -1);
	pop();
	return str;
}

bool LUA::getTableBool (int i)
{
	checkStack();
	lua_rawgeti(_state, -1, i);
	const bool val = lua_toboolean(_state, -1);
	pop();
	return val;
}

int LUA::getTableInteger (int i)
{
	lua_rawgeti(_state, -1, i);
	const int val = lua_tointeger(_state, -1);
	pop();
	return val;
}

float LUA::getTableFloat (int i)
{
	lua_rawgeti(_state, -1, i);
	const float val = lua_tonumber(_state, -1);
	pop();
	return val;
}

std::string LUA::getKey ()
{
	return lua_tostring(_state, -2);
}

void LUA::pop (int amount)
{
	lua_pop(_state, amount);
}

bool LUA::getNextKeyValue ()
{
	return lua_next(_state, -2) != 0;
}

void LUA::getGlobal (const std::string& name)
{
	lua_getglobal(_state, name.c_str());
}

void LUA::debugHook (lua_State *L, lua_Debug *ar)
{
	if (!lua_getinfo(L, "Sn", ar))
		return;

	info(LOG_LUA, String::format("%s %s: %s %d", ar->namewhat, ar->name, ar->short_src, ar->currentline));
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
