#include "tests/TestShared.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "common/Enum.h"

namespace caveexpress {

TEST(EnumSpriteTypesTest, testEnums) {
	ASSERT_EQ(1u, SpriteTypes::WATERFALL.id);
	ASSERT_TRUE(SpriteTypes::isWaterFall(SpriteTypes::WATERFALL));
	ASSERT_EQ(SpriteTypes::WATERFALL, SpriteTypes::WATERFALL);
}

TEST(EnumSpriteTypesTest, testServerOnlyEntityTypes) {
	EXPECT_TRUE(EntityTypes::isServerOnly(EntityTypes::PLATFORM));
	EXPECT_TRUE(EntityTypes::isServerOnly(EntityTypes::WATER));
	EXPECT_TRUE(EntityTypes::isServerOnly(EntityTypes::BORDER));
	EXPECT_TRUE(EntityTypes::isServerOnly(EntityTypes::MODIFICATOR));
	EXPECT_TRUE(EntityTypes::isServerOnly(EntityTypes::EMITTER));
	EXPECT_FALSE(EntityTypes::isServerOnly(EntityTypes::PLAYER));
	EXPECT_FALSE(EntityTypes::isServerOnly(EntityTypes::PACKAGE_ROCK));
	EXPECT_FALSE(EntityTypes::isServerOnly(EntityTypes::SOLID));
	EXPECT_FALSE(EntityTypes::isServerOnly(EntityTypes::CAVE));
}

}
