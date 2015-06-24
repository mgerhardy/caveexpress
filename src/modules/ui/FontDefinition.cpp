#include "FontDefinition.h"
#include "common/Log.h"
#include "common/LUA.h"
#include "common/ExecutionTime.h"
#include "common/System.h"

FontDefinition::FontDefinition() {
	ExecutionTime e("Font definition loading");
	LUA lua;

	if (!lua.load("fonts.lua")) {
		System.exit("could not load fonts", 1);
		return;
	}

	lua.getGlobalKeyValue("fonts");

	while (lua.getNextKeyValue()) {
		const std::string id = lua.getKey();
		if (id.empty()) {
			lua.pop();
			continue;
		}

		FontDefMapConstIter findIter = _fontDefs.find(id);
		if (findIter != _fontDefs.end()) {
			Log::error(LOG_GENERAL, "font def already defined: %s", id.c_str());
			lua.pop();
			continue;
		}

		const int height = lua.getValueIntegerFromTable("height");

		// push the metrics table
		lua.getTable("metrics");
		const int metricsHeight = lua.getValueIntegerFromTable("height");
		const int metricsAscender = lua.getValueIntegerFromTable("ascender");
		const int metricsDescender = lua.getValueIntegerFromTable("descender");
		// pop the metrics table
		lua.pop();

		FontDef *def = new FontDef(id, height, metricsHeight, metricsAscender, metricsDescender);

		// push the chars table
		const int chars = lua.getTable("chars");
		for (int i = 0; i < chars; ++i) {
			lua_pushinteger(lua.getState(), i + 1);
			lua_gettable(lua.getState(), -2);
			// push the char entry
			const std::string character = lua.getValueStringFromTable("char");
			const int width = lua.getValueIntegerFromTable("width");
			const int x = lua.getValueIntegerFromTable("x");
			const int y = lua.getValueIntegerFromTable("y");
			const int w = lua.getValueIntegerFromTable("w");
			const int h = lua.getValueIntegerFromTable("h");
			const int ox = lua.getValueIntegerFromTable("ox");
			const int oy = lua.getValueIntegerFromTable("oy");
			const FontChar c(character, width, x, y, w, h, ox, oy);
			def->fontChars.push_back(c);
			// pop the char entry
			lua.pop();
		}
		// pop the chars table
		lua.pop();

		// push the texture table
		lua.getTable("texture");
		def->textureHeight = lua.getValueIntegerFromTable("height");
		def->textureWidth = lua.getValueIntegerFromTable("width");
		def->textureName = lua.getValueStringFromTable("file").str();
		// pop the texture table
		lua.pop();

		lua.pop();

		_fontDefs[id] = FontDefPtr(def);
	}
}

void FontDef::updateChars (int tWidth, int tHeight)
{
	_widthFactor = tWidth / (float)textureWidth;
	_heightFactor = tHeight / (float)textureHeight;

	for (std::vector<FontChar>::iterator i = fontChars.begin(); i != fontChars.end(); ++i) {
		FontChar& c = *i;
		c.setWidthFactor(_widthFactor);
		c.setHeightFactor(_heightFactor);
	}
}

const FontChar* FontDef::getFontChar (char character)
{
	if (_fontCharMap.empty()) {
		for (std::vector<FontChar>::iterator i = fontChars.begin(); i != fontChars.end(); ++i) {
			_fontCharMap[i->getCharacter()] = &(*i);
		}
	}

	const std::string c(&character, 1);
	std::map<std::string, FontChar*>::iterator iter = _fontCharMap.find(c);
	if (iter == _fontCharMap.end())
		return nullptr;

	return iter->second;
}
