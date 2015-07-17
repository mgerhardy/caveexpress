#include "tests/TestShared.h"
#include "miniracer/shared/MiniRacerEntityType.h"
#include "miniracer/shared/MiniRacerAnimation.h"
#include "miniracer/shared/MiniRacerSpriteType.h"
#include "common/Log.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "ui/FontDefinition.h"
#include "common/EntityType.h"
#include "common/Animation.h"

namespace miniracer {

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
	const std::string spriteId = SpriteDefinition::get().getSpriteName(EntityTypes::SOLID, Animations::IDLE);
	SpriteDefPtr sprite = SpriteDefinition::get().getSpriteDefinition(spriteId);
	ASSERT_TRUE(sprite.get()) << "sprite " + spriteId + " wasn't found";
}

}
