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

}
