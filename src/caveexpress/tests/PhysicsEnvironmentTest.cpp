#include "PhysicsTest.h"
#include "caveexpress/server/entities/Gate.h"
#include "caveexpress/server/entities/PressurePlate.h"
#include "caveexpress/server/entities/npcs/NPCBlowing.h"
#include "caveexpress/shared/CaveExpressAnimation.h"

namespace caveexpress {

TEST_F(PhysicsTest, PressurePlatePlayerActivates)
{
	PressurePlate* plate = static_cast<PressurePlate*>(_map.addTileScripted("tile-plate-01-idle", 8.0f, 10.0f));
	ASSERT_NE(nullptr, plate);
	Player* player = addPlayer(8.0f, 8.5f);
	ASSERT_NE(nullptr, player);
	player->setLinearVelocity(PhysicsVec2(0.0f, 3.0f));
	tick(30);
	EXPECT_TRUE(plate->isActive());
}

TEST_F(PhysicsTest, PressurePlateStoneActivates)
{
	PressurePlate* plate = static_cast<PressurePlate*>(_map.addTileScripted("tile-plate-01-idle", 8.0f, 10.0f));
	ASSERT_NE(nullptr, plate);
	addStone(8.0f, 8.5f, 0.0f, 4.0f);
	tick(30);
	EXPECT_TRUE(plate->isActive());
}

TEST_F(PhysicsTest, PressurePlateNpcDoesNotActivate)
{
	PressurePlate* plate = static_cast<PressurePlate*>(_map.addTileScripted("tile-plate-01-idle", 8.0f, 10.0f));
	ASSERT_NE(nullptr, plate);
	addGroundRow(10.0f, 6, 12);
	NPCAttacking* npc = _map.createAttackingNPC(PhysicsVec2(8.0f, 9.2f), EntityTypes::NPC_WALKING, true);
	ASSERT_NE(nullptr, npc);
	tick(20);
	EXPECT_FALSE(plate->isActive());
}

TEST_F(PhysicsTest, PressurePlateOpensLinkedGate)
{
	Gate* gate = static_cast<Gate*>(_map.addTileScripted("tile-gate-rock-01", 12.0f, 8.0f));
	PressurePlate* plate = static_cast<PressurePlate*>(_map.addTileScripted("tile-plate-01-idle", 8.0f, 10.0f));
	ASSERT_NE(nullptr, gate);
	ASSERT_NE(nullptr, plate);
	plate->setLinkedGate(gate);
	ASSERT_FALSE(gate->isOpenRequested());
	addStone(8.0f, 8.5f, 0.0f, 4.0f);
	tick(30);
	EXPECT_TRUE(plate->isActive());
	EXPECT_TRUE(gate->isOpenRequested());
}

TEST_F(PhysicsTest, OpenGateDoesNotCollideWithDynamic)
{
	Gate* gate = static_cast<Gate*>(_map.addTileScripted("tile-gate-rock-01", 8.0f, 8.0f));
	ASSERT_NE(nullptr, gate);
	gate->setOpen(true);
	tick(40);
	Stone dummy(_map, 0.0f, 0.0f, nullptr);
	EXPECT_FALSE(gate->shouldCollide(&dummy));
}

TEST_F(PhysicsTest, ClosedGateCollidesWithDynamic)
{
	Gate* gate = static_cast<Gate*>(_map.addTileScripted("tile-gate-rock-01", 8.0f, 8.0f));
	ASSERT_NE(nullptr, gate);
	Stone dummy(_map, 0.0f, 0.0f, nullptr);
	EXPECT_TRUE(gate->shouldCollide(&dummy));
}

TEST_F(PhysicsTest, BlowingNpcWindPushesStone)
{
	NPCBlowing* npc = _map.createBlowingNPC(PhysicsVec2(6.0f, 8.0f), true, 20.0f, 3.0f);
	ASSERT_NE(nullptr, npc);
	Stone* stone = addStone(8.0f, 8.0f, 0.0f, 0.0f);
	const float x0 = stone->getPos().x;
	tick(40);
	EXPECT_GT(stone->getPos().x, x0);
}

TEST_F(PhysicsTest, PressurePlateHoldsAfterLeave)
{
	PressurePlate* plate = static_cast<PressurePlate*>(_map.addTileScripted("tile-plate-01-idle", 8.0f, 10.0f));
	ASSERT_NE(nullptr, plate);
	plate->setHoldMs(400);
	Stone* stone = addStone(8.0f, 8.5f, 0.0f, 4.0f);
	tick(30);
	ASSERT_TRUE(plate->isActive());
	freeze(stone, PhysicsVec2(2.0f, 2.0f));
	tick(10);
	EXPECT_TRUE(plate->isActive());
	tick(40);
	EXPECT_FALSE(plate->isActive());
}

TEST_F(PhysicsTest, BombExplodesMovesNearbyStone)
{
	ASSERT_NE(nullptr, _map.addTileScripted("tile-packagetarget-rock-01-idle", 2.0f, 12.0f));
	Bomb* bomb = addBomb(8.0f, 8.0f);
	bomb->setGravityScale(0.0f);
	Stone* stone = addStone(8.6f, 8.0f, 0.0f, 0.0f);
	stone->setGravityScale(0.0f);
	const PhysicsVec2 start = stone->getPos();
	bomb->initiateDetonation();
	tick(140);
	const PhysicsVec2 delta = stone->getPos() - start;
	EXPECT_GT(delta.length(), 0.05f);
}

}
