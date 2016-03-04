#include "TestShared.h"
#include "common/Log.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "ui/FontDefinition.h"
#include "common/EntityType.h"
#include "common/Animation.h"

class LUATest: public AbstractTest {
};

TEST_F(LUATest, testKeyValueMap)
{
	LUA lua;
	const char *buf = "settings = { a = false, b = true, c = 1, d = \"foo\", }";
	ASSERT_TRUE(lua.loadBuffer(buf, "testGlobal"));
	ASSERT_EQ(0, lua.stackCount());

	std::map<std::string, std::string> map;
	lua.getKeyValueMap(map, "settings");

	ASSERT_EQ(4u, map.size());
	ASSERT_NE(map.find("a"), map.end());
	ASSERT_NE(map.find("b"), map.end());
	ASSERT_NE(map.find("c"), map.end());
	ASSERT_NE(map.find("d"), map.end());

	ASSERT_EQ("false", map["a"]);
	ASSERT_EQ("true", map["b"]);
	ASSERT_EQ("1", map["c"]);
	ASSERT_EQ("foo", map["d"]);
}

TEST_F(LUATest, testGlobal)
{
	LUA lua;

	const char *buf = "settings = { a = false, b = true, c = 1, }";
	ASSERT_TRUE(lua.loadBuffer(buf, "testGlobal"));
	ASSERT_EQ(0, lua.stackCount());
	ASSERT_TRUE(lua.getGlobalKeyValue("settings"));
	// we expect settings and nil
	ASSERT_EQ(2, lua.stackCount()) << lua.getStackDump();
	int i = 0;
	while (lua.getNextKeyValue()) {
		const std::string& id = lua.getLuaValue(-2);
		const std::string& value = lua.getLuaValue(-1);
		lua.pop();
		SCOPED_TRACE(string::toString(i));
		if (id == "a") {
			ASSERT_EQ("false", value);
		} else if (id == "b") {
			ASSERT_EQ("true", value);
		} else if (id == "c") {
			ASSERT_EQ("1", value);
		} else {
			EXPECT_TRUE(false) << "id is: " << id;
		}
		++i;
	}
	ASSERT_EQ(1, lua.stackCount()) << lua.getStackDump();
	lua.pop();
	ASSERT_EQ(0, lua.stackCount()) << lua.getStackDump();
	ASSERT_EQ(3, i);
}

TEST_F(LUATest, testLoadFile) {
	LUA lua;
	ASSERT_TRUE(lua.load("fonts.lua"));
	ASSERT_TRUE(lua.getGlobalKeyValue("fonts"));
}

TEST_F(LUATest, testFontDefinition)
{
	FontDefinition d;
	ASSERT_FALSE(d.begin() == d.end()) << "no fonts found";
	// 5 font definitions
	ASSERT_EQ((int)std::distance(d.begin(), d.end()), 5);
	for (auto i = d.begin(); i != d.end(); ++i) {
		const FontDefPtr& def = i->second;
		ASSERT_EQ(def->getFontChar('%'), nullptr);
		ASSERT_EQ(def->getFontChar('a')->getCharacter(), 'a');
		ASSERT_EQ(def->getFontChar('A')->getCharacter(), 'A');
		ASSERT_EQ(def->getFontChar(' ')->getCharacter(), ' ');
		ASSERT_EQ(def->getFontChar('1')->getCharacter(), '1');
		ASSERT_NE(def->getFontChar('a')->getH(), 0);
		ASSERT_NE(def->getFontChar('a')->getW(), 0);
		ASSERT_NE(def->textureHeight, 0);
		ASSERT_NE(def->textureWidth, 0);
		ASSERT_NE(def->textureName, "");
		ASSERT_NE(def->id, "");
	}
}

TEST_F(LUATest, testTextureDefinition)
{
	TextureDefinition small("small");
	ASSERT_FALSE(small.getMap().empty()) << "no texture definitions for small found";
	TextureDefinition big("big");
	ASSERT_FALSE(big.getMap().empty()) << "no texture definitions for big found";
	{
		const TextureDef& td = big.getTextureDef("bones");
		ASSERT_EQ(td.id, "bones");
		ASSERT_EQ(td.textureName, "cavepacker-ui-small");
		ASSERT_FALSE(td.mirror);
		ASSERT_DOUBLE_EQ(td.texcoords.x0, 0.494140625);
		ASSERT_DOUBLE_EQ(td.texcoords.y0, 0.244140625);
		ASSERT_DOUBLE_EQ(td.texcoords.x1, 0.2392578125);
		ASSERT_DOUBLE_EQ(td.texcoords.y1, 0.0400390625);
		ASSERT_EQ(td.trim.trimmedWidth, 245);
		ASSERT_EQ(td.trim.trimmedHeight, 41);
		ASSERT_EQ(td.trim.untrimmedWidth, 245);
		ASSERT_EQ(td.trim.untrimmedHeight, 41);
		ASSERT_EQ(td.trim.trimmedOffsetX, 0);
		ASSERT_EQ(td.trim.trimmedOffsetY, 0);
	}
	{
		const TextureDef& td = big.getTextureDef("gri-campaign");
		ASSERT_EQ(td.id, "gri-campaign");
		ASSERT_EQ(td.textureName, "cavepacker-ui-small");
		ASSERT_FALSE(td.mirror);
	}
}

TEST_F(LUATest, testSpriteDefinition)
{
	TextureDefinition small("small");
	SpriteDefinition::get().init(small);
	SpriteDefPtr spriteDef = SpriteDefinition::get().getSpriteDefinition("test");
	const bool found = !!spriteDef;
	ASSERT_TRUE(found) << "no sprite definitions found for test";
	ASSERT_TRUE(spriteDef->hasShape());
	ASSERT_TRUE(spriteDef->isStatic());
	ASSERT_FALSE(spriteDef->hasNoTextures());
	ASSERT_EQ("test", spriteDef->id);
	ASSERT_DOUBLE_EQ(14, spriteDef->fps);
	ASSERT_DOUBLE_EQ(1, spriteDef->rotateable);
	ASSERT_EQ(2u, spriteDef->polygons.size());
	ASSERT_EQ(1u, spriteDef->circles.size());
	ASSERT_TRUE(spriteDef->textures[LAYER_BACK].empty());
	ASSERT_FALSE(spriteDef->textures[LAYER_MIDDLE].empty());
	ASSERT_TRUE(spriteDef->textures[LAYER_FRONT].empty());
}
