#pragma once

#include <vector>
#include "common/LUAMapContext.h"
#include "CaveTileDefinition.h"
#include "common/SpriteDefinition.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"

namespace caveexpress {

class CaveExpressMapContext: public LUAMapContext {
protected:
	std::vector<CaveTileDefinition> _caveDefinitions;

	bool isSolid (const SpriteType& type) const override {
		return SpriteTypes::isSolid(type);
	}

	static int luaAddCave (lua_State * l)
	{
		CaveExpressMapContext *ctx = reinterpret_cast<CaveExpressMapContext*>(_luaGetContext(l, 1));
		const std::string caveTile = luaL_checkstring(l, 2);
		const gridCoord x = luaL_checknumber(l, 3);
		const gridCoord y = luaL_checknumber(l, 4);
		const EntityType& entityType = EntityType::getByName(luaL_optstring(l, 5, "none"));
		const int delay = luaL_optinteger(l, 6, 5000);

		SpriteDefPtr spriteDefPtr = SpriteDefinition::get().getSpriteDefinition(caveTile);
		if (!spriteDefPtr) {
			Log::info(LOG_GAMEIMPL, "could not add cave: %s", caveTile.c_str());
			ctx->_error = true;
			return 0;
		}

		ctx->addCave(spriteDefPtr, x, y, entityType, delay);

		return 0;
	}

public:
	CaveExpressMapContext(const std::string& name) :
			LUAMapContext(name) {
		luaL_Reg funcs[] = {
				{ "addCave", luaAddCave },
				{ nullptr, nullptr }
		};

		initLUABindings(funcs);
	}

	virtual ~CaveExpressMapContext() {}

	inline const std::vector<CaveTileDefinition>& getCaveTileDefinitions() const {
		return _caveDefinitions;
	}

	inline void setCaveTileDefinitions (const std::vector<CaveTileDefinition>& caveDefinitions) {
		_caveDefinitions = caveDefinitions;
	}

	void addCave(const SpriteDefPtr& spriteDef, gridCoord x, gridCoord y,
			const EntityType& entityType, int delay) {
		const CaveTileDefinition def(x, y, spriteDef, entityType, delay);
		_caveDefinitions.push_back(def);
	}

	virtual bool saveTiles(const FilePtr& file) const override {
		const bool added = LUAMapContext::saveTiles(file);
		if (added && !_caveDefinitions.empty()) {
			file->appendString("\n");
		}
		for (const CaveTileDefinition& i : _caveDefinitions) {
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
		return added || !_caveDefinitions.empty();
	}
};

}
