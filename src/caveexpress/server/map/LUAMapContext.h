#pragma once

#include "engine/common/IMapContext.h"
#include "engine/common/Compiler.h"
#include "engine/common/LUA.h"
#include <string>

struct lua_State;
class SpriteDef;

class LUAMapContext: public IMapContext {
private:
	LUA _lua;
	static LUAMapContext *currentCtx;
	bool _error;

	static int luaGetMapContext (lua_State * l);

	static LUAMapContext* _luaGetContext (lua_State * l, int n);

	static int luaAddCave (lua_State * l);

	static int luaAddTile (lua_State * l);

	static int luaAddEmitter (lua_State * l);

	static int luaSetSetting (lua_State * l);

	// LUA bindings
	void addCave (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, const EntityType& entityType, int delay);
	void addTile (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, EntityAngle angle);
	void addEmitter (const EntityType& type, gridCoord x, gridCoord y, int amount, int delay, const std::string& settings);

public:
	LUAMapContext (const std::string& name);
	virtual ~LUAMapContext ();

	bool isLocationFree (gridCoord x, gridCoord y);

	// IMapContext
	void onMapLoaded () override;
	bool load (bool skipErrors) override;
	bool save () const override { return false; }
};
