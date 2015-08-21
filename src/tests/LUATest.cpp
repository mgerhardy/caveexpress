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

	ASSERT_EQ(3, map.size());
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
	// TODO: it looks like the order is random in lua
	while (lua.getNextKeyValue()) {
		const std::string& id = lua.getLuaValue(-2);
		const std::string& value = lua.getLuaValue(-1);
		lua.pop();
		SCOPED_TRACE(string::toString(i));
		if (i == 0) {
			ASSERT_EQ("a", id);
			ASSERT_EQ("false", value);
		} else if (i == 1) {
			ASSERT_EQ("b", id);
			ASSERT_EQ("true", value);
		} else if (i == 2) {
			ASSERT_EQ("c", id);
			ASSERT_EQ("1", value);
		}
		++i;
	}
	ASSERT_EQ(1, lua.stackCount()) << lua.getStackDump();
	lua.pop();
	ASSERT_EQ(0, lua.stackCount()) << lua.getStackDump();
	ASSERT_EQ(3, i);
}

TEST_F(LUATest, testFontDefinition)
{
	FontDefinition d;
	ASSERT_FALSE(d.begin() == d.end()) << "no fonts found";
}

TEST_F(LUATest, testTextureDefinition)
{
	TextureDefinition small("small");
	ASSERT_FALSE(small.getMap().empty()) << "no texture definitions for small found";
	TextureDefinition big("big");
	ASSERT_FALSE(big.getMap().empty()) << "no texture definitions for big found";
}
