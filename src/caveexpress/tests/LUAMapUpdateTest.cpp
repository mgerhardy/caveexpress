#include "tests/TestShared.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "common/FileSystem.h"
#include "common/LUALibrary.h"
#include "common/TextureDefinition.h"
#include "common/SpriteDefinition.h"
#include <cstring>

namespace caveexpress {

class LUAMapUpdateTest: public AbstractTest {
protected:
	TextureDefinition* _textures = nullptr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		_textures = new TextureDefinition("small");
		SpriteDefinition::get().init(*_textures);
	}

	void TearDown () override
	{
		delete _textures;
		_textures = nullptr;
		AbstractTest::TearDown();
	}
};

TEST_F(LUAMapUpdateTest, testHasFunction)
{
	LUA lua;
	ASSERT_TRUE(lua.loadBuffer(
			"function onUpdate(dt) end\n"
			"notAFunction = 1\n",
			"hasFunction"));
	ASSERT_TRUE(lua.hasFunction("onUpdate"));
	ASSERT_FALSE(lua.hasFunction("missing"));
	ASSERT_FALSE(lua.hasFunction("notAFunction"));
}

TEST_F(LUAMapUpdateTest, testExecuteWithNumberArgument)
{
	LUA lua;
	ASSERT_TRUE(lua.loadBuffer(
			"sum = 0\n"
			"function onUpdate(dt)\n"
			"  sum = sum + dt\n"
			"end\n",
			"executeArg"));
	ASSERT_TRUE(lua.execute("onUpdate", 10.0));
	ASSERT_TRUE(lua.execute("onUpdate", 5.5));
	EXPECT_DOUBLE_EQ(15.5, lua.getFloatValue("sum"));
}

TEST_F(LUAMapUpdateTest, testMapContextOnUpdate)
{
	const char* script =
			"function getName()\n"
			"  return \"onupdate-test\"\n"
			"end\n"
			"updateCalls = 0\n"
			"updateDtSum = 0\n"
			"function onMapLoaded()\n"
			"end\n"
			"function onUpdate(dt)\n"
			"  updateCalls = updateCalls + 1\n"
			"  updateDtSum = updateDtSum + dt\n"
			"end\n"
			"function initMap()\n"
			"  local map = Map.get()\n"
			"  map:setSetting(\"width\", \"2\")\n"
			"  map:setSetting(\"height\", \"2\")\n"
			"  map:setSetting(\"packagetransfercount\", \"1\")\n"
			"  map:setSetting(\"theme\", \"rock\")\n"
			"end\n";

	const std::string name = "lua_onupdate_test";
	const std::string relPath = FS.getDataDir() + FS.getMapsDir() + name + ".lua";
	const std::string absPath = FS.getAbsoluteWritePath() + relPath;
	ASSERT_NE(-1L, FS.writeSysFile(absPath, (const unsigned char*)script, strlen(script), true))
			<< "Failed to write " << absPath;

	class TestableMapContext: public CaveExpressMapContext {
	public:
		explicit TestableMapContext (const std::string& mapName) :
				CaveExpressMapContext(mapName)
		{
		}

		float getLuaFloat (const std::string& global) {
			return _lua.getFloatValue(global);
		}
	};

	TestableMapContext ctx(name);
	ASSERT_TRUE(ctx.load(false));
	ASSERT_TRUE(ctx.hasOnUpdate());
	ASSERT_TRUE(ctx.hasOnMapLoaded());

	ctx.onMapLoaded();
	ctx.onUpdate(16);
	ctx.onUpdate(32);

	EXPECT_FLOAT_EQ(2.0f, ctx.getLuaFloat("updateCalls"));
	EXPECT_FLOAT_EQ(48.0f, ctx.getLuaFloat("updateDtSum"));

	FS.deleteFile(relPath);
}

TEST_F(LUAMapUpdateTest, testMapWithoutOnUpdate)
{
	CaveExpressMapContext ctx("introducing-01-package");
	ASSERT_TRUE(ctx.load(false));
	EXPECT_FALSE(ctx.hasOnUpdate());
	EXPECT_TRUE(ctx.hasOnMapLoaded());
	ctx.onMapLoaded();
	ctx.onUpdate(16); // must be a no-op
}

}
