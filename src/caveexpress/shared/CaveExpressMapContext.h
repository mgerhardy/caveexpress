#pragma once

#include <vector>
#include "common/LUAMapContext.h"
#include "CaveTileDefinition.h"
#include "GateDefinition.h"
#include "PressurePlateDefinition.h"
#include "common/SpriteDefinition.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"

namespace caveexpress {

class Map;

class CaveExpressMapContext: public LUAMapContext {
protected:
	std::vector<CaveTileDefinition> _caveDefinitions;
	std::vector<GateDefinition> _gateDefinitions;
	std::vector<PressurePlateDefinition> _pressurePlateDefinitions;
	Map* _runtimeMap;

	bool isSolid (const SpriteType& type) const override {
		return SpriteTypes::isSolid(type);
	}

	static int luaAddCave (lua_State * l)
	{
		CaveExpressMapContext *ctx = reinterpret_cast<CaveExpressMapContext*>(_luaGetContext(l, 1));
		const std::string caveTile = luaL_checkstring(l, 2);
		const gridCoord x = luaL_checknumber(l, 3);
		const gridCoord y = luaL_checknumber(l, 4);
		const std::string& name = luaL_optstring(l, 5, "none");
		const EntityType& entityType = EntityType::getByName(name);
		if (entityType.isNone() && name != "" && name != "none") {
			Log::error(LOG_COMMON, "invalid entity type given: %s", name.c_str());
			ctx->_error = true;
			return 0;
		}
		const int delay = static_cast<int>(luaL_optinteger(l, 6, 5000));

		SpriteDefPtr spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(caveTile);
		if (!spriteDefPtr) {
			Log::info(LOG_GAMEIMPL, "could not add cave: %s", caveTile.c_str());
			ctx->_error = true;
			return 0;
		}

		ctx->addCave(spriteDefPtr, x, y, entityType, delay);

		return 0;
	}

	static int luaAddGate (lua_State * l)
	{
		CaveExpressMapContext *ctx = reinterpret_cast<CaveExpressMapContext*>(_luaGetContext(l, 1));
		const std::string sprite = luaL_checkstring(l, 2);
		const gridCoord x = luaL_checknumber(l, 3);
		const gridCoord y = luaL_checknumber(l, 4);
		const std::string linkId = luaL_optstring(l, 5, "");
		const float openAmount = static_cast<float>(luaL_optnumber(l, 6, 1.0));

		SpriteDefPtr spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(sprite);
		if (!spriteDefPtr) {
			Log::error(LOG_GAMEIMPL, "could not add gate: %s", sprite.c_str());
			ctx->_error = true;
			return 0;
		}

		ctx->addGate(spriteDefPtr, x, y, linkId, openAmount);
		return 0;
	}

	static int luaAddPressurePlate (lua_State * l)
	{
		CaveExpressMapContext *ctx = reinterpret_cast<CaveExpressMapContext*>(_luaGetContext(l, 1));
		const std::string sprite = luaL_checkstring(l, 2);
		const gridCoord x = luaL_checknumber(l, 3);
		const gridCoord y = luaL_checknumber(l, 4);
		const std::string linkId = luaL_optstring(l, 5, "");
		const float requiredWeight = static_cast<float>(luaL_optnumber(l, 6, 700.0));
		const int holdMs = static_cast<int>(luaL_optinteger(l, 7, 0));

		SpriteDefPtr spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(sprite);
		if (!spriteDefPtr) {
			Log::error(LOG_GAMEIMPL, "could not add pressure plate: %s", sprite.c_str());
			ctx->_error = true;
			return 0;
		}

		ctx->addPressurePlate(spriteDefPtr, x, y, linkId, requiredWeight, holdMs);
		return 0;
	}

public:
	CaveExpressMapContext(const std::string& name) :
			LUAMapContext(name), _runtimeMap(nullptr) {
		luaL_Reg funcs[] = {
				{ "addCave", luaAddCave },
				{ "addGate", luaAddGate },
				{ "addPressurePlate", luaAddPressurePlate },
				{ nullptr, nullptr }
		};

		initLUABindings(funcs);
	}

	virtual ~CaveExpressMapContext() {}

	inline void setRuntimeMap (Map* map) {
		_runtimeMap = map;
	}

	inline Map* getRuntimeMap () const {
		return _runtimeMap;
	}

	inline const std::vector<CaveTileDefinition>& getCaveTileDefinitions() const {
		return _caveDefinitions;
	}

	inline void setCaveTileDefinitions (const std::vector<CaveTileDefinition>& caveDefinitions) {
		_caveDefinitions = caveDefinitions;
	}

	inline const std::vector<GateDefinition>& getGateDefinitions() const {
		return _gateDefinitions;
	}

	inline void setGateDefinitions (const std::vector<GateDefinition>& gateDefinitions) {
		_gateDefinitions = gateDefinitions;
	}

	inline const std::vector<PressurePlateDefinition>& getPressurePlateDefinitions() const {
		return _pressurePlateDefinitions;
	}

	inline void setPressurePlateDefinitions (const std::vector<PressurePlateDefinition>& defs) {
		_pressurePlateDefinitions = defs;
	}

	void addCave(const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y,
			const EntityType& entityType, int delay) {
		const CaveTileDefinition def(x, y, spriteDef, entityType, delay);
		_caveDefinitions.push_back(def);
	}

	void addGate(const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y,
			const std::string& linkId, float openAmount) {
		_gateDefinitions.emplace_back(x, y, spriteDef, linkId, openAmount);
	}

	void addPressurePlate(const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y,
			const std::string& linkId, float requiredWeight, int holdMs) {
		_pressurePlateDefinitions.emplace_back(x, y, spriteDef, linkId, requiredWeight, holdMs);
	}

	virtual bool saveTiles(const FilePtr& file) const override {
		const bool added = LUAMapContext::saveTiles(file);
		bool wroteExtra = false;
		if (added && (!_caveDefinitions.empty() || !_gateDefinitions.empty() || !_pressurePlateDefinitions.empty())) {
			file->appendString("\n");
		}
		for (const CaveTileDefinition& i : _caveDefinitions) {
			wroteExtra = true;
			file->appendString("\tmap:addCave(\"");
			file->appendString(i.spriteDef->id.c_str());
			file->appendString("\", ");
			file->appendString(string::toString(i.x).c_str());
			file->appendString(", ");
			file->appendString(string::toString(i.y).c_str());
			if (!i.type->isNone()) {
				file->appendString(", \"");
				file->appendString(i.type->name.c_str());
				file->appendString("\"");
			}
			if (i.delay > -1) {
				if (i.type->isNone()) {
					file->appendString(", \"");
					file->appendString(i.type->name.c_str());
					file->appendString("\"");
				}
				file->appendString(", ");
				file->appendString(string::toString(i.delay).c_str());
			}
			file->appendString(")\n");
		}
		for (const GateDefinition& i : _gateDefinitions) {
			wroteExtra = true;
			file->appendString("\tmap:addGate(\"");
			file->appendString(i.spriteDef->id.c_str());
			file->appendString("\", ");
			file->appendString(string::toString(i.x).c_str());
			file->appendString(", ");
			file->appendString(string::toString(i.y).c_str());
			file->appendString(", \"");
			file->appendString(i.linkId.c_str());
			file->appendString("\", ");
			file->appendString(string::toString(i.openAmount).c_str());
			file->appendString(")\n");
		}
		for (const PressurePlateDefinition& i : _pressurePlateDefinitions) {
			wroteExtra = true;
			file->appendString("\tmap:addPressurePlate(\"");
			file->appendString(i.spriteDef->id.c_str());
			file->appendString("\", ");
			file->appendString(string::toString(i.x).c_str());
			file->appendString(", ");
			file->appendString(string::toString(i.y).c_str());
			file->appendString(", \"");
			file->appendString(i.linkId.c_str());
			file->appendString("\", ");
			file->appendString(string::toString(i.requiredWeight).c_str());
			file->appendString(", ");
			file->appendString(string::toString(i.holdMs).c_str());
			file->appendString(")\n");
		}
		return added || wroteExtra;
	}
};

}
