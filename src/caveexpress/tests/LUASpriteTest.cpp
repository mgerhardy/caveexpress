#include "tests/TestShared.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "common/Logger.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "ui/FontDefinition.h"
#include "common/EntityType.h"
#include "common/Animation.h"

namespace caveexpress {

class LUASpriteTest: public AbstractTest {
protected:
	virtual void SetUp() override {
		AbstractTest::SetUp();
		TextureDefinition t("small");
		SpriteDefinition::get().init(t);
	}
};

TEST_F(LUASpriteTest, testSpriteDefinition)
{
	const std::string spriteId = SpriteDefinition::get().getSpriteName(EntityTypes::PACKAGETARGET_ROCK,
			Animations::ANIMATION_IDLE);
	SpriteDefPtr sprite = SpriteDefinition::get().getSpriteDefinition(spriteId);
	ASSERT_TRUE(sprite.get()) << "sprite " + spriteId + " wasn't found";
}

}
