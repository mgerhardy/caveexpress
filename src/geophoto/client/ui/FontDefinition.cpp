#include "FontDefinition.h"
#include "shared/FileSystem.h"
#include "shared/Logger.h"
#include "shared/LUA.h"
#include "shared/ExecutionTime.h"
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
			error(LOG_GENERAL, "font def already defined: " + id);
			lua.pop();
			continue;
		}

		FontDef *def = new FontDef(id);
		def->height = lua.getValueIntegerFromTable("height");

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
			const FontChar c = { character, width, x, y, w, h, ox, oy };
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

		// push the metrics table
		lua.getTable("metrics");
		def->metricsHeight = lua.getValueIntegerFromTable("height");
		def->metricsAscender = lua.getValueIntegerFromTable("ascender");
		def->metricsDescender = lua.getValueIntegerFromTable("descender");
		// pop the metrics table
		lua.pop();

		lua.pop();

		_fontDefs[id] = FontDefPtr(def);
	}
}

void FontDef::updateChars (int tWidth, int tHeight)
{
	const float widthFactor = tWidth / (float)textureWidth;
	const float heightFactor = tHeight / (float)textureHeight;

	debug(LOG_CLIENT, string::toString(*this));
	debug(LOG_CLIENT, "tWidth: " + string::toString(tWidth) + ", tHeight: " + string::toString(tHeight)
					+ ", widthFactor: " + string::toString(widthFactor)
					+ ", heightFactor: " + string::toString(heightFactor));
	const float newHeight = this->height * heightFactor;
	this->height = newHeight + 0.5f;
	for (std::vector<FontChar>::iterator i = fontChars.begin(); i != fontChars.end(); ++i) {
		FontChar& c = *i;
		const float nw = c.w * widthFactor;
		const float nh = c.h * heightFactor;
		const float nox = c.ox * widthFactor;
		const float noy = c.oy * heightFactor;
		const float nx = c.x * widthFactor;
		const float ny = c.y * heightFactor;
		const float nwidth = c.width * widthFactor;
		debug(LOG_CLIENT, "before " + string::toString(c));
		c.w = nw + 0.5f;
		c.h = nh + 0.5f;
		c.x = nx;
		c.y = ny;
		c.ox = nox;
		c.oy = noy;
		c.width = nwidth + 0.5f;
		debug(LOG_CLIENT, "after " + string::toString(c));
	}
}

const FontChar* FontDef::getFontChar (char character)
{
	if (_fontCharMap.empty()) {
		for (std::vector<FontChar>::iterator i = fontChars.begin(); i != fontChars.end(); ++i) {
			_fontCharMap[i->character] = &(*i);
		}
	}

	const std::string c(&character, 1);
	std::map<std::string, FontChar*>::iterator iter = _fontCharMap.find(c);
	if (iter == _fontCharMap.end())
		return nullptr;

	return iter->second;
}
