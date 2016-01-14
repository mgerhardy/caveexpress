#pragma once
#include "common/Compiler.h"
#include "common/Config.h"
GCC_DIAG_OFF(cast-qual)
GCC_DIAG_OFF(cast-align)
#include <lua.hpp>
GCC_DIAG_ON(cast-align)
GCC_DIAG_ON(cast-qual)
#include "common/String.h"
#include "common/Log.h"
#include <map>
#include <SDL_platform.h>
#include <SDL_assert.h>

class LUA {
private:
	lua_State *_state;

	static void debugHook (lua_State *L, lua_Debug *ar);
	static int panicHook (lua_State *L);

	static int isAndroid (lua_State *L);
	static int isWindows (lua_State *L);
	static int isMacOSX (lua_State *L);
	static int isIOS (lua_State *L);
	static int isLinux (lua_State *L);
	static int isHTML5 (lua_State *L);
	static int isOUYA (lua_State *L);
	static int isDebug (lua_State *L);
	static int isHD (lua_State *L);
	static int isTouch (lua_State *L);
	static int isNaCl (lua_State *L);
	static int isSteamLink (lua_State *L);

public:
	explicit LUA (bool debug = false);
	~LUA ();

	void init (bool debug = false);
	void close ();

	inline lua_State* getState () const;

	template<class T>
	static T** newUserdata (lua_State *L, const std::string& prefix)
	{
		T ** udata = (T **) lua_newuserdata(L, sizeof(T *));
		const std::string name = "luaL_" + prefix;
		luaL_getmetatable(L, name.c_str());
		lua_setmetatable(L, -2);
		return udata;
	}

	template<class T>
	static T* getUserData (lua_State *L, int n, const std::string& prefix)
	{
		const std::string name = "luaL_" + prefix;
		return *(T **) luaL_checkudata(L, n, name.c_str());
	}

	static void returnError (lua_State *L, const std::string& error)
	{
		luaL_error(L, "%s", error.c_str());
	}

	bool getGlobal (const std::string& name);

	std::string getKey ();

	bool getGlobalKeyValue (const std::string& name);

	/**
	 * @note This will NOT return the values in order
	 * @sa http://www.lua.org/manual/5.1/manual.html#lua_next
	 */
	bool getNextKeyValue ();

	void pop (int amount = 1);

	int stackCount ();

	int getTable (const std::string& name);

	std::string getTableString (int i);

	int getTableInteger (int i);

	bool getTableBool (int i);

	float getTableFloat (int i);

	void reg (const std::string& prefix, luaL_Reg* funcs);

	bool load (const std::string &file);
	bool loadBuffer(const std::string& buffer, const char *ctx);
	/**
	 * @param[in] function function to be called
	 */
	bool execute (const std::string &function, int returnValues = 0);

	char getValueCharFromTable (const char * key, const char defaultValue = '\0');
	std::string getValueStringFromTable (const char * key, const std::string& defaultValue = "");
	float getValueFloatFromTable (const char * key, float defaultValue = 0.0f);
	int getValueIntegerFromTable (const char * key, int defaultValue = 0);
	bool getValueBoolFromTable (const char * key, bool defaultValue = false);
	void getKeyValueMap (std::map<std::string, std::string>& map, const std::string& key);

	int getIntValue (const std::string& xpath, int defaultValue = 0);
	float getFloatValue (const std::string& path, float defaultValue = 0.0f);
	bool getBoolValue (const std::string& xpath);
	std::string getStringFromStack ();
	std::string getString (const std::string& expr, const std::string& defaultValue = "");

	std::string getLuaValue (int i, int depth = 0);
	void tableDump ();
	std::string getStackDump ();
	void stackDump ();
};

inline lua_State* LUA::getState () const
{
	return _state;
}

class LUAStackChecker {
private:
	lua_State *_state;
	const int _startStackDepth;
public:
	explicit LUAStackChecker (lua_State *state) :
			_state(state), _startStackDepth(lua_gettop(_state))
	{
	}
	~LUAStackChecker ()
	{
		SDL_assert_always(_startStackDepth == lua_gettop(_state));
	}
};

#ifdef DEBUG
#define LUA_checkStack() LUAStackChecker(this->_state)
#else
#define LUA_checkStack() do {} while(0)
#endif

#ifdef DEBUG
#define LUA_checkStack2(lua) LUAStackChecker(lua)
#else
#define LUA_checkStack2(lua) do {} while(0)
#endif
