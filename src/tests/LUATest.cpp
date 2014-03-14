#include "tests/LUATest.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "engine/common/Logger.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/common/TextureDefinition.h"
#include "engine/client/ui/FontDefinition.h"
#include "engine/common/EntityType.h"
#include "engine/common/Animation.h"

class LUATest: public MapSuite {
};

TEST_F(LUATest, testSpriteDefinition)
{
	const std::string spriteId = SpriteDefinition::get().getSpriteName(EntityTypes::PACKAGETARGET_ROCK,
			Animations::ANIMATION_IDLE);
	SpriteDefPtr sprite = SpriteDefinition::get().getSpriteDefinition(spriteId);
	ASSERT_TRUE(sprite.get()) << "sprite " + spriteId + " wasn't found";
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
