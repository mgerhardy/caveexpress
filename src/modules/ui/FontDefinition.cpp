#include "FontDefinition.h"
#include "common/Log.h"
#include "common/LUALibrary.h"
#include "common/ExecutionTime.h"
#include "common/System.h"

FontDefinition::FontDefinition() {
	ExecutionTime e("Font definition loading");
	LUA lua;
	Log::debug(LOG_UI, "Font definition loading");

	if (!lua.load("fonts.lua")) {
		Log::error(LOG_UI, "Could not load fonts.lua");
		System.exit("could not load fonts", 1);
		return;
	}

	if (!lua.getGlobalKeyValue("fonts")) {
		Log::error(LOG_UI, "font def: Could not find the global fonts map");
		return;
	}

	while (lua.getNextKeyValue()) {
		LUA_checkStack2(lua.getState());
		const std::string id = lua.getKey();
		if (id.empty()) {
			Log::error(LOG_UI, "font def: no key found in font definition: %s", lua.getStackDump().c_str());
			lua.pop();
			continue;
		}

		FontDefMapConstIter findIter = _fontDefs.find(id);
		if (findIter != _fontDefs.end()) {
			Log::error(LOG_UI, "font def already defined: %s", id.c_str());
			lua.pop();
			continue;
		}

		const int height = lua.getValueIntegerFromTable("height");

		// push the metrics table
		if (lua.getTable("metrics") == -1) {
			Log::error(LOG_UI, "font def %s doesn't have a metrics table", id.c_str());
			continue;
		}

		const int metricsHeight = lua.getValueIntegerFromTable("height");
		const int metricsAscender = lua.getValueIntegerFromTable("ascender");
		const int metricsDescender = lua.getValueIntegerFromTable("descender");
		// pop the metrics table
		lua.pop();

		FontDef *def = new FontDef(id, height, metricsHeight, metricsAscender, metricsDescender);
		FontChars fontChars;

		// push the chars table
		const int chars = lua.getTable("chars");
		Log::debug(LOG_UI, "found %i chars entries", chars);

		if (chars > 0) {
			lua_State* L = lua.getState();
			lua_pushvalue(L, -1);
			lua_pushnil(L);
			while (lua_next(L, -2)) {
				if (!lua_istable(L, -1)) {
					Log::error(LOG_UI, "expected char table on the stack for %s: %s", id.c_str(), lua.getStackDump().c_str());
					lua.pop();
					continue;
				}
				// push the char entry
				const int character = lua.getValueCharFromTable("char");
				const int width = lua.getValueIntegerFromTable("width");
				const int x = lua.getValueIntegerFromTable("x");
				const int y = lua.getValueIntegerFromTable("y");
				const int w = lua.getValueIntegerFromTable("w");
				const int h = lua.getValueIntegerFromTable("h");
				const int ox = lua.getValueIntegerFromTable("ox");
				const int oy = lua.getValueIntegerFromTable("oy");
				FontChar* c = new FontChar(character, width, x, y, w, h, ox, oy);
				fontChars.push_back(c);
				// pop the char entry
				lua.pop();
			}
			lua.pop();
			// pop the chars table
			lua.pop();
		}

		// push the texture table
		if (lua.getTable("texture") != -1) {
			def->textureHeight = lua.getValueIntegerFromTable("height");
			def->textureWidth = lua.getValueIntegerFromTable("width");
			def->textureName = lua.getValueStringFromTable("file");
			// pop the texture table
			lua.pop();
		}

		lua.pop();

		def->init(fontChars);

		_fontDefs[id] = FontDefPtr(def);
	}
	Log::debug(LOG_UI, "Loaded %i font definitions", (int)_fontDefs.size());
}

void FontDef::init (FontChars& fontChars)
{
	SDL_assert(!fontChars.empty());
	for (FontChar* c : fontChars) {
		_fontCharMap[c->getCharacter()] = c;
	}
}

void FontDef::updateChars (int tWidth, int tHeight)
{
	_widthFactor = tWidth / (float)textureWidth;
	_heightFactor = tHeight / (float)textureHeight;

	for (auto entry : _fontCharMap) {
		FontChar* c = entry.second;
		c->setWidthFactor(_widthFactor);
		c->setHeightFactor(_heightFactor);
	}
}

const FontChar* FontDef::getFontChar (int character)
{
	auto iter = _fontCharMap.find(character);
	if (iter == _fontCharMap.end())
		return nullptr;
	const FontChar* fc = iter->second;
	SDL_assert_always(fc != nullptr);
	if (fc->getCharacter() != '\0')
		return fc;

	return nullptr;
}
