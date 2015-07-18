#pragma once

#include "common/IMapContext.h"
#include "common/Compiler.h"
#include "common/LUA.h"
#include "common/FileSystem.h"
#include <string>
struct lua_State;

class SpriteDef;

class LUAMapContext: public IMapContext {
protected:
	LUA _lua;
	static LUAMapContext *currentCtx;
	bool _error;

	static int luaGetMapContext (lua_State * l);

	static LUAMapContext* _luaGetContext (lua_State * l, int n);

	static int luaAddTile (lua_State * l);

	static int luaAddEmitter (lua_State * l);

	static int luaSetSetting (lua_State * l);

	static int luaAddStartPosition (lua_State * l);

	virtual bool isSolid (const SpriteType& type) const = 0;

	void initLUABindings (luaL_Reg* additional);

	// LUA bindings
	virtual void addTile (const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y, EntityAngle angle);
	virtual void addEmitter (const EntityType& type, gridCoord x, gridCoord y, int amount, int delay, const std::string& settings);

	virtual void saveTiles(const FilePtr& file) const;

public:
	explicit LUAMapContext (const std::string& name);
	virtual ~LUAMapContext ();

	bool isLocationFree (gridCoord x, gridCoord y);

	// IMapContext
	virtual void onMapLoaded () override;
	virtual bool load (bool skipErrors) override;
	virtual bool save () const override;
};
