#include "tests/TestShared.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "common/Log.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "ui/FontDefinition.h"
#include "common/EntityType.h"
#include "common/Animation.h"

namespace caveexpress {

class LUASpriteTest: public AbstractTest {
private:
	TextureDefinition *_t;
protected:
	virtual void SetUp() override {
		AbstractTest::SetUp();
		_t = new TextureDefinition("small");
		SpriteDefinition::get().init(*_t);
	}
	virtual void TearDown() override {
		AbstractTest::TearDown();
		delete _t;
		_t = nullptr;
	}
};

TEST_F(LUASpriteTest, testSpriteDefinition)
{
	const std::string spriteId = SpriteDefinition::get().getSpriteName(EntityTypes::PACKAGETARGET_ROCK,
			Animations::ANIMATION_IDLE);
	SpriteDefPtr sprite = SpriteDefinition::get().getSpriteDefinition(spriteId);
	ASSERT_TRUE(sprite.get()) << "sprite " + spriteId + " wasn't found";
}

TEST_F(LUASpriteTest, testSpriteLight)
{
	const SpriteDefPtr cave = SpriteDefinition::get().getSpriteDefinition("tile-cave-01");
	ASSERT_TRUE(cave.get());
	EXPECT_TRUE(cave->emitsLight());
	EXPECT_NEAR(6.0f, cave->lightRadius, 0.001f);
	EXPECT_NEAR(1.0f, cave->lightIntensity, 0.001f);
	EXPECT_NEAR(0.35f, cave->lightOffsetY, 0.001f);
	EXPECT_NEAR(2.0f, cave->lightFalloff, 0.001f);

	const SpriteDefPtr window = SpriteDefinition::get().getSpriteDefinition("tile-background-window-01");
	ASSERT_TRUE(window.get());
	EXPECT_TRUE(window->emitsLight());
	EXPECT_NEAR(4.0f, window->lightRadius, 0.001f);
	EXPECT_NEAR(0.7f, window->lightIntensity, 0.001f);

	const SpriteDefPtr lava = SpriteDefinition::get().getSpriteDefinition("tile-lava-rock-left-01");
	ASSERT_TRUE(lava.get());
	EXPECT_TRUE(lava->emitsLight());
	EXPECT_NEAR(3.5f, lava->lightRadius, 0.001f);
	EXPECT_NEAR(1.0f, lava->lightR, 0.001f);
	EXPECT_NEAR(0.22f, lava->lightG, 0.001f);

	const SpriteDefPtr rock = SpriteDefinition::get().getSpriteDefinition("tile-rock-01");
	if (rock) {
		EXPECT_FALSE(rock->emitsLight());
	}
}

}
