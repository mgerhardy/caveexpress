#pragma once

#include "caveexpress/shared/ICaveMapContext.h"
#include "common/Compiler.h"
#include "common/LUA.h"
#include <string>
struct lua_State;

namespace caveexpress {

class SpriteDef;

class CaveExpressMapContext: public ICaveMapContext {
private:
	LUA _lua;
	static CaveExpressMapContext *currentCtx;
	bool _error;

	static int luaGetMapContext (lua_State * l);

	static CaveExpressMapContext* _luaGetContext (lua_State * l, int n);

	static int luaAddCave (lua_State * l);

	static int luaAddTile (lua_State * l);

	static int luaAddEmitter (lua_State * l);

	static int luaSetSetting (lua_State * l);

	static int luaAddStartPosition (lua_State * l);

	// LUA bindings
	void addCave (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, const EntityType& entityType, int delay);
	void addTile (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, EntityAngle angle);
	void addEmitter (const EntityType& type, gridCoord x, gridCoord y, int amount, int delay, const std::string& settings);

public:
	explicit CaveExpressMapContext (const std::string& name);
	virtual ~CaveExpressMapContext ();

	bool isLocationFree (gridCoord x, gridCoord y);

	// IMapContext
	void onMapLoaded () override;
	bool load (bool skipErrors) override;
	bool save () const override { return false; }
};

}
