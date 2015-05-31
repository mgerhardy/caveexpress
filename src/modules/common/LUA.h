#pragma once
#include "common/Compiler.h"
#include "common/Config.h"
GCC_DIAG_OFF(cast-qual)
GCC_DIAG_OFF(cast-align)
#ifdef HAVE_LUA_H
extern "C" {
#endif
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#ifdef HAVE_LUA_H
}
#endif
GCC_DIAG_ON(cast-align)
GCC_DIAG_ON(cast-qual)
#include "common/String.h"
#include "common/Logger.h"
#include "common/FileSystem.h"
#include <map>
#include <SDL_platform.h>

class LUA {
private:
	lua_State *_state;

	static void debugHook (lua_State *L, lua_Debug *ar);

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

public:
	LUA (bool debug = false);
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

	void getGlobal (const std::string& name);

	std::string getKey ();

	void getGlobalKeyValue (const std::string& name);

	bool getNextKeyValue ();

	void pop (int amount = 1);

	int getTable (const std::string& name);

	std::string getTableString (int i);

	int getTableInteger (int i);

	bool getTableBool (int i);

	float getTableFloat (int i);

	void reg (const std::string& prefix, luaL_Reg* funcs);

	bool load (const std::string &file);
	/**
	 * @param[in] function function to be called
	 */
	bool execute (const String &function, int returnValues = 0);

	String getValueStringFromTable (const char * key, const String& defaultValue = "");
	float getValueFloatFromTable (const char * key, float defaultValue = 0.0f);
	int getValueIntegerFromTable (const char * key, int defaultValue = 0);
	bool getValueBoolFromTable (const char * key, bool defaultValue = false);
	void getKeyValueMap (std::map<std::string, std::string>& map, const char *key);

	int getIntValue (const std::string& xpath, int defaultValue = 0);
	float getFloatValue (const std::string& path, float defaultValue = 0.0f);
	bool getBoolValue (const std::string& xpath);
	std::string getStringFromStack ();
	String getString (const std::string& expr, const std::string& defaultValue = "");

	void stackDump ();
};

inline lua_State* LUA::getState () const
{
	return _state;
}
