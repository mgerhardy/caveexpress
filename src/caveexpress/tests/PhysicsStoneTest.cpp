#include "PhysicsTest.h"

namespace caveexpress {

TEST_F(PhysicsTest, StoneFallingOnIdleTreeDropsFruit)
{
	Tree* tree = addTree(8.0f, 8.0f);
	addStone(8.0f, 5.5f, 0.0f, 4.0f);
	tick(20);
	EXPECT_EQ(TreeState::TREE_DAZED, tree->getState());
	EXPECT_EQ(1, countFruit());
}

TEST_F(PhysicsTest, DazedTreeDoesNotDropAgain)
{
	Tree* tree = addTree(8.0f, 8.0f);
	addStone(8.0f, 5.5f, 0.0f, 4.0f);
	tick(20);
	ASSERT_EQ(1, countFruit());
	addStone(8.2f, 5.5f, 0.0f, 4.0f);
	tick(20);
	EXPECT_EQ(TreeState::TREE_DAZED, tree->getState());
	EXPECT_EQ(1, countFruit());
}

TEST_F(PhysicsTest, IdleTreeDropsAgainAfterCooldown)
{
	Tree* tree = addTree(8.0f, 8.0f);
	addStone(8.0f, 5.5f, 0.0f, 4.0f);
	tick(20);
	ASSERT_EQ(1, countFruit());
	tick(320);
	EXPECT_EQ(TreeState::TREE_IDLE, tree->getState());
	addStone(8.0f, 5.5f, 0.0f, 4.0f);
	tick(20);
	EXPECT_EQ(2, countFruit());
}

TEST_F(PhysicsTest, EmptyTreeDazesWithoutFruit)
{
	Tree* tree = addTree(8.0f, 8.0f);
	tick(4);
	for (int i = 0; i < 10; ++i) {
		Stone* stone = addStone(1.0f + static_cast<float>(i) * 0.15f, 1.0f, 0.0f, 0.0f);
		stone->setGravityScale(0.0f);
		tree->setDazed(stone);
		tick(4);
		ASSERT_EQ(i + 1, countFruit()) << "fruit " << (i + 1);
		tree->setIdle();
	}
	addStone(8.0f, 5.5f, 0.0f, 4.0f);
	tick(20);
	EXPECT_EQ(TreeState::TREE_DAZED, tree->getState());
	EXPECT_EQ(10, countFruit());
}

TEST_F(PhysicsTest, StoneNotFallingDoesNotDazeTree)
{
	Tree* tree = addTree(8.0f, 8.0f);
	addStone(8.0f, 8.0f, 0.0f, 0.0f);
	tick(20);
	EXPECT_EQ(TreeState::TREE_IDLE, tree->getState());
	EXPECT_EQ(0, countFruit());
}

TEST_F(PhysicsTest, FallingStoneDestroysPackage)
{
	ASSERT_NE(nullptr, _map.addTileScripted("tile-packagetarget-rock-01-idle", 2.0f, 12.0f));
	Package* package = addPackage(8.0f, 8.0f);
	package->setLinearVelocity(PhysicsVec2_zero);
	addStone(8.0f, 6.0f, 0.0f, 5.0f);
	tick(30);
	EXPECT_TRUE(package->isDestroyed());
}

TEST_F(PhysicsTest, FallingStoneDazesWalkingNpc)
{
	addGroundRow(10.0f, 6, 12);
	NPCAttacking* npc = _map.createAttackingNPC(PhysicsVec2(8.0f, 9.2f), EntityTypes::NPC_WALKING, true);
	ASSERT_NE(nullptr, npc);
	addStone(8.0f, 7.0f, 0.0f, 6.0f);
	tick(25);
	EXPECT_TRUE(npc->isDazed());
}

TEST_F(PhysicsTest, StoneAtRestDoesNotDazeWalkingNpc)
{
	addGroundRow(10.0f, 6, 12);
	NPCAttacking* npc = _map.createAttackingNPC(PhysicsVec2(8.0f, 9.2f), EntityTypes::NPC_WALKING, true);
	ASSERT_NE(nullptr, npc);
	Stone* stone = addStone(npc->getPos().x, npc->getPos().y, 0.0f, 0.0f);
	freeze(stone, npc->getPos());
	tick(20);
	EXPECT_FALSE(npc->isDazed());
}

}
