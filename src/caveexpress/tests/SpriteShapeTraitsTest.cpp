#include "tests/TestShared.h"
#include "caveexpress/shared/SpriteShapeTraits.h"
#include "caveexpress/server/map/RandomMapGenerator.h"
#include "common/ThemeType.h"
#include "common/TextureDefinition.h"
#include "common/SpriteDefinition.h"

namespace caveexpress {

class SpriteShapeTraitsTest: public AbstractTest {
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

	SpriteDefPtr requireSprite (const char* id) const
	{
		const SpriteDefPtr def = SpriteDefinition::get().getSpriteDefinition(id);
		EXPECT_TRUE(!!def) << "missing sprite fixture " << id;
		return def;
	}
};

TEST_F(SpriteShapeTraitsTest, testFullSolidFromDefaultBox)
{
	const SpriteDefPtr def = requireSprite("tile-rock-01");
	ASSERT_TRUE(!!def);
	const SpriteShapeTraits t = analyzeSpriteShape(def);
	EXPECT_TRUE(t.fullSolid);
	EXPECT_FALSE(t.shim);
	EXPECT_FALSE(t.thinTopSlab);
	EXPECT_FALSE(t.undercutL);
	EXPECT_FALSE(t.undercutR);
}

TEST_F(SpriteShapeTraitsTest, testThinTopSlabAcrossThemes)
{
	const char* fixtures[] = {
		"tile-ground-05",
		"tile-ground-ice-05",
		"tile-ground-jungle-05",
		"tile-ground-desert-05"
	};
	for (const char* id : fixtures) {
		const SpriteDefPtr def = requireSprite(id);
		ASSERT_TRUE(!!def) << id;
		const SpriteShapeTraits t = analyzeSpriteShape(def);
		EXPECT_TRUE(t.thinTopSlab) << id;
		EXPECT_FALSE(t.fullSolid) << id;
		EXPECT_FALSE(t.shim) << id;
	}
}

TEST_F(SpriteShapeTraitsTest, testShimAcrossThemes)
{
	const char* fixtures[] = {
		"tile-rock-shim-01",
		"tile-rock-shim-ice-01",
		"tile-rock-shim-jungle-01",
		"tile-rock-shim-desert-01"
	};
	for (const char* id : fixtures) {
		const SpriteDefPtr def = requireSprite(id);
		ASSERT_TRUE(!!def) << id;
		const SpriteShapeTraits t = analyzeSpriteShape(def);
		EXPECT_TRUE(t.shim) << id;
		EXPECT_FALSE(t.fullSolid) << id;
		EXPECT_FALSE(t.undercutL) << id;
		EXPECT_FALSE(t.undercutR) << id;
	}
}

TEST_F(SpriteShapeTraitsTest, testUndercutsAcrossThemes)
{
	struct Case {
		const char* id;
		bool left;
	};
	const Case fixtures[] = {
		{ "tile-rock-slope-left-02", true },
		{ "tile-rock-slope-right-02", false },
		{ "tile-rock-slope-ice-left-02", true },
		{ "tile-rock-slope-ice-right-02", false },
		{ "tile-rock-slope-jungle-left-02", true },
		{ "tile-rock-slope-jungle-right-02", false },
		{ "tile-rock-slope-desert-left-02", true },
		{ "tile-rock-slope-desert-right-02", false }
	};
	for (const Case& c : fixtures) {
		const SpriteDefPtr def = requireSprite(c.id);
		ASSERT_TRUE(!!def) << c.id;
		const SpriteShapeTraits t = analyzeSpriteShape(def);
		EXPECT_FALSE(t.fullSolid) << c.id;
		EXPECT_FALSE(t.shim) << c.id;
		EXPECT_FALSE(t.slopeL) << c.id;
		EXPECT_FALSE(t.slopeR) << c.id;
		if (c.left) {
			EXPECT_TRUE(t.undercutL) << c.id;
			EXPECT_FALSE(t.undercutR) << c.id;
		} else {
			EXPECT_TRUE(t.undercutR) << c.id;
			EXPECT_FALSE(t.undercutL) << c.id;
		}
	}
}

TEST_F(SpriteShapeTraitsTest, testWalkableSlopesAcrossThemes)
{
	struct Case {
		const char* id;
		bool left;
	};
	const Case fixtures[] = {
		{ "tile-rock-slope-left-01", true },
		{ "tile-rock-slope-right-01", false },
		{ "tile-rock-slope-ice-left-01", true },
		{ "tile-rock-slope-ice-right-01", false },
		{ "tile-rock-slope-jungle-left-01", true },
		{ "tile-rock-slope-jungle-right-01", false },
		{ "tile-rock-slope-desert-left-01", true },
		{ "tile-rock-slope-desert-right-01", false }
	};
	for (const Case& c : fixtures) {
		const SpriteDefPtr def = requireSprite(c.id);
		ASSERT_TRUE(!!def) << c.id;
		const SpriteShapeTraits t = analyzeSpriteShape(def);
		EXPECT_FALSE(t.fullSolid) << c.id;
		EXPECT_FALSE(t.shim) << c.id;
		EXPECT_FALSE(t.undercutL) << c.id;
		EXPECT_FALSE(t.undercutR) << c.id;
		if (c.left) {
			EXPECT_TRUE(t.slopeL) << c.id;
			EXPECT_FALSE(t.slopeR) << c.id;
		} else {
			EXPECT_TRUE(t.slopeR) << c.id;
			EXPECT_FALSE(t.slopeL) << c.id;
		}
	}
}

TEST_F(SpriteShapeTraitsTest, testThemeBucketsHaveStructuralTiles)
{
	const ThemeType* themes[] = {
		&ThemeTypes::ROCK, &ThemeTypes::ICE, &ThemeTypes::JUNGLE, &ThemeTypes::DESERT
	};
	for (const ThemeType* theme : themes) {
		RandomMapGenerator gen(*theme, 18, 12);
		const RandomMapGenerator::SpriteBucketCounts c = gen.spriteBucketCounts();
		EXPECT_GT(c.rocksFull, 0) << theme->name;
		EXPECT_GT(c.grounds, 0) << theme->name;
		EXPECT_GT(c.groundsHanging, 0) << theme->name;
		EXPECT_GT(c.undercutL, 0) << theme->name;
		EXPECT_GT(c.undercutR, 0) << theme->name;
		EXPECT_GT(c.shims, 0) << theme->name;
		EXPECT_GT(c.slopesL, 0) << theme->name;
		EXPECT_GT(c.slopesR, 0) << theme->name;
		EXPECT_GT(c.backgrounds, 0) << theme->name;
		EXPECT_GT(c.caves, 0) << theme->name;
	}
}

TEST_F(SpriteShapeTraitsTest, testAllThemesGenerate)
{
	RandomMapRules rules = RandomMapRules::loadFromLua();
	rules.caveTarget = 2;
	const ThemeType* themes[] = {
		&ThemeTypes::ROCK, &ThemeTypes::ICE, &ThemeTypes::JUNGLE, &ThemeTypes::DESERT
	};
	for (const ThemeType* theme : themes) {
		RandomMapGenerator gen(*theme, 18, 12, rules);
		EXPECT_TRUE(gen.generate(4242u).success) << theme->name;
	}
}

}
