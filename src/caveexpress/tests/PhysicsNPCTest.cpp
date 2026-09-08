#include "PhysicsTest.h"
#include "caveexpress/server/entities/CaveMapTile.h"

namespace caveexpress {

TEST_F(PhysicsTest, AttackingWalkingNpcCrashesPlayer)
{
	addGroundRow(10.0f, 4, 14);
	Player* player = addPlayer(8.0f, 8.5f);
	ASSERT_NE(nullptr, player);
	tick(2);
	NPCAttacking* npc = _map.createAttackingNPC(player->getPos(), EntityTypes::NPC_WALKING, false);
	ASSERT_NE(nullptr, npc);
	npc->setState(NPCState::NPC_ATTACKING);
	npc->setLinearVelocity(PhysicsVec2(-3.0f, 0.0f));
	tick(12);
	EXPECT_TRUE(player->isCrashed());
	EXPECT_EQ(CRASH_NPC_WALKING, player->getCrashReason());
}

TEST_F(PhysicsTest, NonAttackingWalkingNpcDoesNotCrashPlayer)
{
	addGroundRow(10.0f, 4, 14);
	Player* player = addPlayer(8.0f, 9.0f);
	ASSERT_NE(nullptr, player);
	player->setLinearVelocity(PhysicsVec2_zero);
	NPCAttacking* npc = _map.createAttackingNPC(PhysicsVec2(9.5f, 9.2f), EntityTypes::NPC_WALKING, false);
	ASSERT_NE(nullptr, npc);
	npc->setState(NPCState::NPC_IDLE);
	npc->setLinearVelocity(PhysicsVec2(-2.0f, 0.0f));
	tick(20);
	EXPECT_FALSE(player->isCrashed());
}

TEST_F(PhysicsTest, FlyingNpcCrashesPlayer)
{
	Player* player = addPlayer(8.0f, 5.0f);
	ASSERT_NE(nullptr, player);
	tick(2);
	NPCFlying* npc = _map.createFlyingNPC(player->getPos());
	ASSERT_NE(nullptr, npc);
	tick(12);
	EXPECT_TRUE(player->isCrashed());
	EXPECT_EQ(CRASH_NPC_FLYING, player->getCrashReason());
}

TEST_F(PhysicsTest, DyingFlyingNpcDoesNotCrashPlayer)
{
	Player* player = addPlayer(8.0f, 5.0f);
	ASSERT_NE(nullptr, player);
	const PhysicsVec2 pos = player->getPos();
	NPCFlying* npc = _map.createFlyingNPC(pos);
	ASSERT_NE(nullptr, npc);
	npc->setDying(nullptr);
	freeze(player, pos);
	freeze(npc, pos);
	tick(8);
	EXPECT_FALSE(player->isCrashed());
}

TEST_F(PhysicsTest, FishNpcCrashesPlayerInWater)
{
	Player* player = addPlayer(8.0f, 14.5f);
	ASSERT_NE(nullptr, player);
	tick(2);
	NPCFish* npc = _map.createFishNPC(player->getPos());
	ASSERT_NE(nullptr, npc);
	tick(12);
	EXPECT_TRUE(player->isCrashed());
	EXPECT_EQ(CRASH_NPC_FISH, player->getCrashReason());
}

TEST_F(PhysicsTest, FriendlyMovingNpcBoardsPlayer)
{
	addGroundRow(10.0f, 3, 12);
	CaveMapTile* cave = static_cast<CaveMapTile*>(_map.addTileScripted("tile-cave-01", 5.0f, 9.0f));
	ASSERT_NE(nullptr, cave);
	Player* player = addPlayer(7.0f, 9.0f);
	ASSERT_NE(nullptr, player);
	player->setLinearVelocity(PhysicsVec2_zero);
	NPCFriendly* npc = _map.spawnFriendlyNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN, false);
	ASSERT_NE(nullptr, npc);
	npc->setMoving(player->getPos());
	npc->setPos(player->getPos());
	tick(40);
	EXPECT_TRUE(player->isTransfering(npc) || npc->getState() == NPCState::NPC_COLLECTED);
}

TEST_F(PhysicsTest, FriendlyIdleFastPlayerKnocksOff)
{
	addGroundRow(10.0f, 3, 12);
	CaveMapTile* cave = static_cast<CaveMapTile*>(_map.addTileScripted("tile-cave-01", 5.0f, 9.0f));
	ASSERT_NE(nullptr, cave);
	Player* player = addPlayer(8.0f, 5.0f);
	ASSERT_NE(nullptr, player);
	tick(2);
	NPCFriendly* npc = _map.spawnFriendlyNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN, false);
	ASSERT_NE(nullptr, npc);
	npc->setIdle();
	npc->setPos(player->getPos());
	player->setLinearVelocity(PhysicsVec2(4.5f, 0.2f));
	tick(50);
	EXPECT_TRUE(npc->isFalling() || npc->isSwimming() || npc->isStruggle() || npc->isDying());
}

TEST_F(PhysicsTest, AggressiveNpcsIgnoreCrashedPlayer)
{
	_serviceProvider.updateNetwork(true);
	addGroundRow(10.0f, 4, 14);
	Player* wreck = addPlayer(8.0f, 8.5f);
	Player* living = _map.spawnPlayerAt(12.0f, 8.5f, 2);
	ASSERT_NE(nullptr, wreck);
	ASSERT_NE(nullptr, living);
	tick(1);
	wreck->setCrashed(CRASH_DAMAGE);
	wreck->setLives(0);
	ASSERT_FALSE(wreck->isLive());

	NPCFlying* bird = _map.createFlyingNPC(wreck->getPos());
	ASSERT_NE(nullptr, bird);
	EXPECT_FALSE(bird->shouldCollide(wreck));
	EXPECT_TRUE(bird->shouldCollide(living));

	NPCFish* fish = _map.createFishNPC(PhysicsVec2(8.0f, 14.0f));
	ASSERT_NE(nullptr, fish);
	EXPECT_FALSE(fish->shouldCollide(wreck));
	EXPECT_TRUE(fish->shouldCollide(living));

	NPCAttacking* walker = _map.createAttackingNPC(wreck->getPos(), EntityTypes::NPC_WALKING, false);
	ASSERT_NE(nullptr, walker);
	EXPECT_FALSE(walker->shouldCollide(wreck));
	EXPECT_TRUE(walker->shouldCollide(living));
	walker->setState(NPCState::NPC_ATTACKING);
	tick(8);
	EXPECT_FALSE(living->isCrashed());
	EXPECT_TRUE(wreck->isCrashed());
}

TEST_F(PhysicsTest, FallingStoneKillsFlyingNpc)
{
	NPCFlying* npc = _map.createFlyingNPC(PhysicsVec2(8.0f, 6.0f));
	ASSERT_NE(nullptr, npc);
	addStone(8.0f, 4.5f, 0.0f, 8.0f);
	tick(25);
	EXPECT_TRUE(npc->isDying());
}

}
