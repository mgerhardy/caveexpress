#include "TestShared.h"
#include "common/Log.h"
#include "common/SpriteDefinition.h"
#include "common/SpritePolygonLua.h"
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
		EXPECT_EQ(td.id, "bones");
		EXPECT_EQ(td.textureName, "cavepacker-ui-big");
		EXPECT_FALSE(td.mirror);
		EXPECT_DOUBLE_EQ(td.texcoords.x0, 0);
		EXPECT_DOUBLE_EQ(td.texcoords.y0, 0.9027780294418335);
		EXPECT_DOUBLE_EQ(td.texcoords.x1, 0.58894199132919312);
		EXPECT_DOUBLE_EQ(td.texcoords.y1, 0.09375);
		EXPECT_EQ(td.trim.trimmedWidth, 490);
		EXPECT_EQ(td.trim.trimmedHeight, 81);
		EXPECT_EQ(td.trim.untrimmedWidth, 490);
		EXPECT_EQ(td.trim.untrimmedHeight, 81);
		EXPECT_EQ(td.trim.trimmedOffsetX, 0);
		EXPECT_EQ(td.trim.trimmedOffsetY, 0);
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

TEST_F(LUATest, testSpritePolygonLuaRoundTrip)
{
	std::vector<SpritePolygon> polygons;
	SpritePolygon apple("");
	apple.vertices.push_back(SpriteVertex(0.16f, 0.0f));
	apple.vertices.push_back(SpriteVertex(0.09f, 0.11f));
	apple.vertices.push_back(SpriteVertex(-0.09f, 0.13f));
	polygons.push_back(apple);
	SpritePolygon tagged("solid");
	tagged.vertices.push_back(SpriteVertex(-0.5f, 0.5f));
	tagged.vertices.push_back(SpriteVertex(0.5f, 0.5f));
	tagged.vertices.push_back(SpriteVertex(0.5f, 0.2f));
	tagged.vertices.push_back(SpriteVertex(-0.5f, 0.2f));
	polygons.push_back(tagged);

	const std::string lua = sprite_polygon_lua::toLua(polygons);
	ASSERT_NE(std::string::npos, lua.find("polygons = {"));
	ASSERT_NE(std::string::npos, lua.find("\"solid\""));

	std::vector<SpritePolygon> parsed;
	std::string error;
	ASSERT_TRUE(sprite_polygon_lua::fromLua(lua, parsed, &error)) << error;
	ASSERT_EQ(2u, parsed.size());
	ASSERT_EQ("", parsed[0].userData);
	ASSERT_EQ("solid", parsed[1].userData);
	ASSERT_EQ(3u, parsed[0].vertices.size());
	ASSERT_EQ(4u, parsed[1].vertices.size());
	EXPECT_NEAR(0.16f, parsed[0].vertices[0].x, 1.0e-4f);
	EXPECT_NEAR(0.11f, parsed[0].vertices[1].y, 1.0e-4f);
	EXPECT_NEAR(-0.5f, parsed[1].vertices[0].x, 1.0e-4f);
	EXPECT_NEAR(0.2f, parsed[1].vertices[3].y, 1.0e-4f);

	std::vector<SpritePolygon> single;
	ASSERT_TRUE(sprite_polygon_lua::fromLua("{ \"\", 16.0, 0.0, 9.0, 11.0 }", single, &error)) << error;
	ASSERT_EQ(1u, single.size());
	ASSERT_EQ(2u, single[0].vertices.size());
	EXPECT_NEAR(0.16f, single[0].vertices[0].x, 1.0e-5f);
	EXPECT_NEAR(0.09f, single[0].vertices[1].x, 1.0e-5f);
}
