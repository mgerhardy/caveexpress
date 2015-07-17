#pragma once

#include "common/IMapContext.h"
#include "common/Compiler.h"
#include "common/LUA.h"
#include <string>
struct lua_State;

namespace miniracer {

class SpriteDef;

class MiniRacerMapContext: public IMapContext {
private:
	LUA _lua;
	static MiniRacerMapContext *currentCtx;
	bool _error;

	static int luaGetMapContext (lua_State * l);

	static MiniRacerMapContext* _luaGetContext (lua_State * l, int n);

	static int luaAddTile (lua_State * l);

	static int luaAddEmitter (lua_State * l);

	static int luaSetSetting (lua_State * l);

	static int luaAddStartPosition (lua_State * l);

	// LUA bindings
	void addTile (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, EntityAngle angle);
	void addEmitter (const EntityType& type, gridCoord x, gridCoord y, int amount, int delay, const std::string& settings);

public:
	explicit MiniRacerMapContext (const std::string& name);
	virtual ~MiniRacerMapContext ();

	bool isLocationFree (gridCoord x, gridCoord y);

	// IMapContext
	void onMapLoaded () override;
	bool load (bool skipErrors) override;
	bool save () const override { return false; }
};

}
