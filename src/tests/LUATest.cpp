#include "TestShared.h"
#include "common/Log.h"
#include "common/SpriteDefinition.h"
#include "common/SpritePolygonLua.h"
#include "common/SpriteLuaPatcher.h"
#include "common/TextureDefinition.h"
#include "ui/FontDefinition.h"
#include "common/EntityType.h"
#include "common/Animation.h"
#include "common/LUALibrary.h"
#include <fstream>

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

TEST_F(LUATest, testSpriteCircleLuaRoundTrip)
{
	std::vector<SpriteCircle> circles;
	SpriteCircle bomb("fuse");
	bomb.center = SpriteVertex(0.0f, -0.25f);
	bomb.radius = 0.23f;
	circles.push_back(bomb);

	const std::string lua = sprite_polygon_lua::toLuaCircles(circles);
	ASSERT_NE(std::string::npos, lua.find("circles = {"));
	ASSERT_NE(std::string::npos, lua.find("\"fuse\""));

	std::vector<SpriteCircle> parsed;
	std::string error;
	ASSERT_TRUE(sprite_polygon_lua::fromLuaCircles(lua, parsed, &error)) << error;
	ASSERT_EQ(1u, parsed.size());
	EXPECT_EQ("fuse", parsed[0].userData);
	EXPECT_NEAR(0.0f, parsed[0].center.x, 1.0e-4f);
	EXPECT_NEAR(-0.25f, parsed[0].center.y, 1.0e-4f);
	EXPECT_NEAR(0.23f, parsed[0].radius, 1.0e-4f);
}

TEST_F(LUATest, testSpriteLuaPatcher)
{
	const std::string src =
			"sprites = {\n"
			"\t[\"item-bomb-idle\"] = {\n"
			"\t\trotateable = 1,\n"
			"\t\tcircles = {\n"
			"\t\t\t{ \"\", 0, -25, 23 },\n"
			"\t\t},\n"
			"\t},\n"
			"\t[\"item-bomb-explode\"] = {},\n"
			"}\n";
	std::vector<SpritePolygon> polys;
	std::vector<SpriteCircle> circles;
	SpriteCircle c("");
	c.center = SpriteVertex(0.1f, -0.2f);
	c.radius = 0.3f;
	circles.push_back(c);
	std::string out;
	std::string error;
	ASSERT_TRUE(sprite_lua_patcher::patchSpriteShapes(src, "item-bomb-idle", polys, circles, out, &error)) << error;
	ASSERT_NE(std::string::npos, out.find("circles = {"));
	ASSERT_NE(std::string::npos, out.find("rotateable = 1"));
	ASSERT_EQ(std::string::npos, out.find("{ \"\", 0, -25, 23 }"));
	LUA lua;
	ASSERT_TRUE(lua.loadBuffer(out, "patched-bomb"));
}

TEST_F(LUATest, testSpriteLuaPatcherInsertWithoutTrailingComma)
{
	const std::string src =
			"sprites = {\n"
			"\t[\"player-flying\"] = {\n"
			"\t\tfps = 18\n"
			"\t},\n"
			"}\n";
	std::vector<SpritePolygon> polys;
	SpritePolygon poly("");
	poly.vertices.push_back(SpriteVertex(0.16f, 0.0f));
	poly.vertices.push_back(SpriteVertex(0.09f, 0.11f));
	poly.vertices.push_back(SpriteVertex(-0.09f, 0.13f));
	polys.push_back(poly);
	std::string out;
	std::string error;
	ASSERT_TRUE(sprite_lua_patcher::patchSpriteShapes(src, "player-flying", polys, {}, out, &error)) << error;
	ASSERT_NE(std::string::npos, out.find("fps = 18,"));
	ASSERT_NE(std::string::npos, out.find("polygons = {"));
	LUA lua;
	ASSERT_TRUE(lua.loadBuffer(out, "patched-insert"));
}

TEST_F(LUATest, testSpriteLuaPatcherEraseAndIsolate)
{
	const std::string src =
			"-- keep this comment\n"
			"sprites = {\n"
			"\t[\"item-package\"] = {\n"
			"\t\tpolygons = {\n"
			"\t\t\t{ \"\", 1, 2, 3, 4, 5, 6 },\n"
			"\t\t},\n"
			"\t},\n"
			"\t[\"item-package-ice\"] = {\n"
			"\t\tpolygons = {\n"
			"\t\t\t{ \"\", 9, 9, 8, 8, 7, 7 },\n"
			"\t\t},\n"
			"\t},\n"
			"}\n";
	std::string out;
	std::string error;
	ASSERT_TRUE(sprite_lua_patcher::patchSpriteShapes(src, "item-package", {}, {}, out, &error)) << error;
	ASSERT_NE(std::string::npos, out.find("-- keep this comment"));
	ASSERT_NE(std::string::npos, out.find("[\"item-package\"]"));
	ASSERT_NE(std::string::npos, out.find("[\"item-package-ice\"]"));
	ASSERT_NE(std::string::npos, out.find("{ \"\", 9, 9, 8, 8, 7, 7 }"));
	size_t open = 0;
	size_t close = 0;
	ASSERT_TRUE(sprite_lua_patcher::findSpriteTable(out, "item-package", open, close));
	size_t keyStart = 0;
	size_t valueClose = 0;
	EXPECT_FALSE(sprite_lua_patcher::findTableKey(out, open, close, "polygons", keyStart, valueClose));
	LUA lua;
	ASSERT_TRUE(lua.loadBuffer(out, "patched-erase"));
}

TEST_F(LUATest, testSpriteLuaPatcherRealSpritesLua)
{
	std::ifstream in;
	const char* paths[] = {
		"base/caveexpress/sprites.lua",
		"../base/caveexpress/sprites.lua",
		"../../base/caveexpress/sprites.lua"
	};
	for (const char* path : paths) {
		in.open(path);
		if (in.good())
			break;
		in.close();
	}
	ASSERT_TRUE(in.good()) << "base/caveexpress/sprites.lua";
	const std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	ASSERT_FALSE(src.empty());

	std::vector<SpritePolygon> apple;
	ASSERT_TRUE(sprite_polygon_lua::fromLua(
			"polygons = { { \"\", 16.0, 0.0, 9.0, 11.0, -9.0, 13.0, -15.0, 0.0, -8.0, -12.0, 9.0, -12.0, 0.0, -16.0, }, }",
			apple, nullptr));
	apple[0].vertices[0].x = 0.20f;

	std::string out;
	std::string error;
	ASSERT_TRUE(sprite_lua_patcher::patchSpriteShapes(src, "item-apple-idle", apple, {}, out, &error)) << error;
	ASSERT_NE(std::string::npos, out.find("rotateable = 1"));
	const size_t banana = out.find("[\"item-banana-idle\"]");
	ASSERT_NE(std::string::npos, banana);
	ASSERT_NE(std::string::npos, out.find("-0.855, -1.04, -19.8, 0.238", banana));

	std::vector<SpriteCircle> bomb;
	SpriteCircle c("");
	c.center = SpriteVertex(0.0f, -0.25f);
	c.radius = 0.30f;
	bomb.push_back(c);
	std::string out2;
	ASSERT_TRUE(sprite_lua_patcher::patchSpriteShapes(out, "item-bomb-idle", {}, bomb, out2, &error)) << error;
	ASSERT_NE(std::string::npos, out2.find("30.0"));

	LUA lua;
	ASSERT_TRUE(lua.loadBuffer(out2, "real-sprites-patched"));
	ASSERT_TRUE(lua.getGlobalKeyValue("sprites"));
}

TEST_F(LUATest, testSpriteLuaPatcherRoundTripAndErrors)
{
	const std::string src =
			"sprites = {\n"
			"\t-- polygons = { should stay a comment }\n"
			"\t[\"item-apple-idle\"] = {\n"
			"\t\ttype = \"collectable\",\n"
			"\t\tframes = {\n"
			"\t\t\t{}, --back\n"
			"\t\t\t{ \"item-apple-idle\", }, --middle\n"
			"\t\t},\n"
			"\t\tpolygons = {\n"
			"\t\t\t{ \"\", 16.0, 0.0, 9.0, 11.0, -9.0, 13.0 },\n"
			"\t\t},\n"
			"\t},\n"
			"}\n";

	std::string error;
	std::string out;
	ASSERT_FALSE(sprite_lua_patcher::patchSpriteShapes(src, "does-not-exist", {}, {}, out, &error));
	EXPECT_NE(std::string::npos, error.find("not found"));

	std::vector<SpritePolygon> polys;
	SpritePolygon poly("solid");
	poly.vertices.push_back(SpriteVertex(0.10f, 0.20f));
	poly.vertices.push_back(SpriteVertex(-0.10f, 0.20f));
	poly.vertices.push_back(SpriteVertex(0.00f, -0.15f));
	polys.push_back(poly);
	std::vector<SpriteCircle> circles;
	SpriteCircle circle("fuse");
	circle.center = SpriteVertex(0.05f, -0.25f);
	circle.radius = 0.12f;
	circles.push_back(circle);

	ASSERT_TRUE(sprite_lua_patcher::patchSpriteShapes(src, "item-apple-idle", polys, circles, out, &error)) << error;
	EXPECT_NE(std::string::npos, out.find("-- polygons = { should stay a comment }"));
	EXPECT_NE(std::string::npos, out.find("type = \"collectable\""));
	EXPECT_NE(std::string::npos, out.find("{ \"item-apple-idle\", }, --middle"));

	size_t tableOpen = 0;
	size_t tableClose = 0;
	ASSERT_TRUE(sprite_lua_patcher::findSpriteTable(out, "item-apple-idle", tableOpen, tableClose));
	const std::string body = out.substr(tableOpen, tableClose - tableOpen + 1);
	std::vector<SpritePolygon> parsedPolys;
	std::vector<SpriteCircle> parsedCircles;
	ASSERT_TRUE(sprite_polygon_lua::fromLuaShapes(body, parsedPolys, parsedCircles, &error)) << error;
	ASSERT_EQ(1u, parsedPolys.size());
	ASSERT_EQ(3u, parsedPolys[0].vertices.size());
	EXPECT_EQ("solid", parsedPolys[0].userData);
	EXPECT_NEAR(0.10f, parsedPolys[0].vertices[0].x, 1.0e-4f);
	EXPECT_NEAR(0.20f, parsedPolys[0].vertices[0].y, 1.0e-4f);
	EXPECT_NEAR(0.00f, parsedPolys[0].vertices[2].x, 1.0e-4f);
	EXPECT_NEAR(-0.15f, parsedPolys[0].vertices[2].y, 1.0e-4f);
	ASSERT_EQ(1u, parsedCircles.size());
	EXPECT_EQ("fuse", parsedCircles[0].userData);
	EXPECT_NEAR(0.05f, parsedCircles[0].center.x, 1.0e-4f);
	EXPECT_NEAR(-0.25f, parsedCircles[0].center.y, 1.0e-4f);
	EXPECT_NEAR(0.12f, parsedCircles[0].radius, 1.0e-4f);

	LUA lua;
	ASSERT_TRUE(lua.loadBuffer(out, "patched-roundtrip"));
}

TEST_F(LUATest, testFindAssignmentTable)
{
	const std::string src =
			"-- npcwalking = { leftover comment }\n"
			"npcwalking = {\n"
			"	width = 1.95,\n"
			"	height = 0.8,\n"
			"}\n"
			"player = {\n"
			"	width = 0.94,\n"
			"	height = 0.87,\n"
			"}\n";
	size_t open = 0;
	size_t close = 0;
	ASSERT_TRUE(sprite_lua_patcher::findAssignmentTable(src, "npcwalking", open, close));
	const std::string body = src.substr(open, close - open + 1);
	EXPECT_NE(std::string::npos, body.find("width = 1.95"));
	EXPECT_EQ(std::string::npos, body.find("player"));
	ASSERT_TRUE(sprite_lua_patcher::findAssignmentTable(src, "player", open, close));
	EXPECT_NE(std::string::npos, src.substr(open, close - open + 1).find("0.94"));
	EXPECT_FALSE(sprite_lua_patcher::findAssignmentTable(src, "npcwalk", open, close));
}
