#include "EnumSpriteTypesTest.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "common/Enum.h"

TEST(EnumSpriteTypesTest, testEnums) {
	ASSERT_EQ(1, SpriteTypes::WATERFALL.id);
	ASSERT_TRUE(SpriteTypes::isWaterFall(SpriteTypes::WATERFALL));
	ASSERT_EQ(SpriteTypes::WATERFALL, SpriteTypes::WATERFALL);
}
