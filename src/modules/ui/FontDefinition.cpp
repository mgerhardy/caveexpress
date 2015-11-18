#include "FontDefinition.h"
#include "common/Log.h"
#include "common/LUA.h"
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
		FontChars fontChars(128);

		// push the chars table
		const int chars = lua.getTable("chars");
		Log::debug(LOG_UI, "found %i chars entries", chars);
		for (int i = 0; i < chars; ++i) {
			lua_pushinteger(lua.getState(), i + 1);
			lua_gettable(lua.getState(), -2);
			if (!lua_istable(lua.getState(), -1)) {
				Log::error(LOG_UI, "expected char table on the stack for %s: %s", id.c_str(), lua.getStackDump().c_str());
				lua.pop();
				continue;
			}
			// push the char entry
			const char character = lua.getValueCharFromTable("char");
			const int width = lua.getValueIntegerFromTable("width");
			const int x = lua.getValueIntegerFromTable("x");
			const int y = lua.getValueIntegerFromTable("y");
			const int w = lua.getValueIntegerFromTable("w");
			const int h = lua.getValueIntegerFromTable("h");
			const int ox = lua.getValueIntegerFromTable("ox");
			const int oy = lua.getValueIntegerFromTable("oy");
			const FontChar c(character, width, x, y, w, h, ox, oy);
			fontChars.push_back(c);
			// pop the char entry
			lua.pop();
		}
		// pop the chars table
		if (chars != -1)
			lua.pop();

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

void FontDef::init (const FontChars& fontChars)
{
	SDL_assert(!fontChars.empty());
	for (const FontChar& c : fontChars) {
		_fontCharMap[(int)c.getCharacter()] = c;
	}
}

void FontDef::updateChars (int tWidth, int tHeight)
{
	_widthFactor = tWidth / (float)textureWidth;
	_heightFactor = tHeight / (float)textureHeight;

	const int len = SDL_arraysize(_fontCharMap);
	for (int i = 0; i < len; ++i) {
		FontChar& c = _fontCharMap[i];
		c.setWidthFactor(_widthFactor);
		c.setHeightFactor(_heightFactor);
	}
}

const FontChar* FontDef::getFontChar (char character)
{
	SDL_assert_always(character >= 0);
	SDL_assert_always(character <= SDL_arraysize(_fontCharMap));
	const FontChar& fc = _fontCharMap[(int)character];
	if (fc.getCharacter() != '\0')
		return &fc;

	return nullptr;
}
