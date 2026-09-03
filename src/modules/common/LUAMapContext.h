#pragma once

#include "common/IMapContext.h"
#include "common/Compiler.h"
#include "common/LUALibrary.h"
#include "common/FileSystem.h"
#include <string>
struct lua_State;

class SpriteDef;

class LUAMapContext: public IMapContext {
protected:
	LUA _lua;
	static LUAMapContext *currentCtx;
	bool _error;
	bool _hasOnMapLoaded;
	bool _hasOnUpdate;
	/**
	 * Top-level Lua kept across editor saves (onMapLoaded/onUpdate/helpers/locals).
	 * @c getName and @c initMap are regenerated from structured map data.
	 */
	std::string _preservedLogic;

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

	virtual bool saveTiles(const FilePtr& file) const;

	/** Capture non-initMap/non-getName source so editor saves do not strip cutscene logic. */
	void capturePreservedLogic (const std::string& source);

public:
	explicit LUAMapContext (const std::string& name);
	virtual ~LUAMapContext ();

	bool isLocationFree (gridCoord x, gridCoord y);

	LUA& getLua ();
	const LUA& getLua () const;

	bool hasOnUpdate () const;
	bool hasOnMapLoaded () const;
	const std::string& getPreservedLogic () const;
	void setPreservedLogic (const std::string& logic);

	// IMapContext
	virtual void onMapLoaded () override;
	virtual void onUpdate (uint32_t deltaTime) override;
	virtual bool load (bool skipErrors) override;
	virtual bool save () const override;
	virtual const std::string& getScriptLogic () const override;
	virtual void setScriptLogic (const std::string& logic) override;
};

inline LUA& LUAMapContext::getLua ()
{
	return _lua;
}

inline const LUA& LUAMapContext::getLua () const
{
	return _lua;
}

inline bool LUAMapContext::hasOnUpdate () const
{
	return _hasOnUpdate;
}

inline bool LUAMapContext::hasOnMapLoaded () const
{
	return _hasOnMapLoaded;
}

inline const std::string& LUAMapContext::getPreservedLogic () const
{
	return _preservedLogic;
}

inline void LUAMapContext::setPreservedLogic (const std::string& logic)
{
	_preservedLogic = logic;
}

inline const std::string& LUAMapContext::getScriptLogic () const
{
	return _preservedLogic;
}

inline void LUAMapContext::setScriptLogic (const std::string& logic)
{
	_preservedLogic = logic;
}
