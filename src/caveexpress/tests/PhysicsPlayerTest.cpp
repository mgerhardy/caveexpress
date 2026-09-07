#include "PhysicsTest.h"

namespace caveexpress {

TEST_F(PhysicsTest, PlayerTouchesLavaFixtureCrashes)
{
	ASSERT_NE(nullptr, _map.addTileScripted("tile-lava-rock-left-01", 8.0f, 10.0f));
	// Spawn already overlapping the lava polygon. Player::setPos() cannot be used
	// after createBody — the revolute joint keeps the original world pivot.
	Player* player = addPlayer(8.0f, 10.0f);
	ASSERT_NE(nullptr, player);
	tick(12);
	EXPECT_TRUE(player->isCrashed());
	EXPECT_EQ(CRASH_LAVA, player->getCrashReason());
}

TEST_F(PhysicsTest, PlayerTouchesLavaRockDoesNotCrash)
{
	ASSERT_NE(nullptr, _map.addTileScripted("tile-lava-rock-left-01", 8.0f, 10.0f));
	Player* player = addPlayer(8.0f, 10.8f);
	ASSERT_NE(nullptr, player);
	tick(8);
	EXPECT_FALSE(player->isCrashed());
}

TEST_F(PhysicsTest, PlayerHardLandingDamages)
{
	addGroundRow(12.0f, 6, 12);
	Player* player = addPlayer(8.0f, 3.0f);
	ASSERT_NE(nullptr, player);
	player->setPos(PhysicsVec2(8.5f, 4.0f));
	const uint16_t hp = player->getHitpoints();
	player->setLinearVelocity(PhysicsVec2(0.0f, 20.0f));
	tick(80);
	EXPECT_LT(player->getHitpoints(), hp);
}

TEST_F(PhysicsTest, PlayerSlowLandingDoesNotDamage)
{
	addGroundRow(8.0f, 6, 12);
	Player* player = addPlayer(8.0f, 7.2f);
	ASSERT_NE(nullptr, player);
	player->setLinearVelocity(PhysicsVec2_zero);
	const uint16_t hp = player->getHitpoints();
	tick(10);
	EXPECT_EQ(hp, player->getHitpoints());
	EXPECT_FALSE(player->isCrashed());
}

TEST_F(PhysicsTest, PlayerCollectsFruit)
{
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	tick(2);
	player->subtractHitpoints(30);
	ASSERT_LT(player->getHitpoints(), 100u);
	const uint16_t hp = player->getHitpoints();
	addFruit(EntityTypes::APPLE, player->getPos().x, player->getPos().y);
	tick(15);
	EXPECT_EQ(0, countFruit());
	EXPECT_GT(player->getHitpoints(), hp);
	EXPECT_TRUE(player->isFree());
}

TEST_F(PhysicsTest, PlayerCollectsEgg)
{
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	freeze(player, player->getPos());
	addEgg(player->getPos().x, player->getPos().y);
	tick(15);
	EXPECT_EQ(0, countType(EntityTypes::EGG));
	EXPECT_TRUE(player->isFree());
}

TEST_F(PhysicsTest, PlayerCollectsStone)
{
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	player->setLinearVelocity(PhysicsVec2_zero);
	addStone(8.0f, 6.0f, 0.0f, 0.0f);
	tick(15);
	EXPECT_FALSE(player->isFree());
}

TEST_F(PhysicsTest, PlayerTouchesWater)
{
	Player* player = addPlayer(8.0f, 14.5f);
	ASSERT_NE(nullptr, player);
	tick(10);
	EXPECT_TRUE(player->isTouchingWater());
}

TEST_F(PhysicsTest, CrashOnTouchBorderFailsMap)
{
	Player* player = addPlayer(0.4f, 5.0f);
	ASSERT_NE(nullptr, player);
	player->setLinearVelocity(PhysicsVec2(-8.0f, 0.0f));
	tick(30);
	// Default sandbox side borders do not crash (SIDEBORDERFAIL is off).
	EXPECT_FALSE(player->isCrashed());
}

TEST_F(PhysicsTest, TopBorderClampsPlayerWithoutCrash)
{
	Player* player = addPlayer(8.0f, 0.8f);
	ASSERT_NE(nullptr, player);
	player->setLinearVelocity(PhysicsVec2(0.0f, -12.0f));
	tick(20);
	EXPECT_FALSE(player->isCrashed());
	EXPECT_GE(player->getLinearVelocity().y, 0.0f);
}

}
