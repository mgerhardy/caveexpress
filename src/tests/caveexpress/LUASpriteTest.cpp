#include "LUASpriteTest.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "engine/common/Logger.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/common/TextureDefinition.h"
#include "engine/client/ui/FontDefinition.h"
#include "engine/common/EntityType.h"
#include "engine/common/Animation.h"

class LUASpriteTest: public MapSuite {
};

TEST_F(LUASpriteTest, testSpriteDefinition)
{
	const std::string spriteId = SpriteDefinition::get().getSpriteName(EntityTypes::PACKAGETARGET_ROCK,
			Animations::ANIMATION_IDLE);
	SpriteDefPtr sprite = SpriteDefinition::get().getSpriteDefinition(spriteId);
	ASSERT_TRUE(sprite.get()) << "sprite " + spriteId + " wasn't found";
}
