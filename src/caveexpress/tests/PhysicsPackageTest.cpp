#include "PhysicsTest.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/PackageTarget.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/constants/NPCState.h"

namespace caveexpress {

TEST_F(PhysicsTest, PackageArrivesAtMatchingTarget)
{
	MapTile* tile = _map.addTileScripted("tile-packagetarget-rock-01-idle", 8.0f, 10.0f);
	ASSERT_NE(nullptr, tile);
	Package* package = addPackage(8.5f, 9.0f);
	package->setLinearVelocity(PhysicsVec2(0.0f, 3.0f));
	tick(120);
	EXPECT_EQ(1, _map.getDeliveredPackageCount());
}

TEST_F(PhysicsTest, AttackingNpcDestroysPackage)
{
	ASSERT_NE(nullptr, _map.addTileScripted("tile-packagetarget-rock-01-idle", 2.0f, 12.0f));
	addGroundRow(10.0f, 6, 12);
	Package* package = addPackage(8.0f, 9.0f);
	package->setLinearVelocity(PhysicsVec2_zero);
	NPCAttacking* npc = _map.createAttackingNPC(PhysicsVec2(8.0f, 9.2f), EntityTypes::NPC_WALKING, true);
	ASSERT_NE(nullptr, npc);
	npc->setState(NPCState::NPC_ATTACKING);
	tick(25);
	EXPECT_TRUE(package->isDestroyed());
}

TEST_F(PhysicsTest, PackageNpcDoesNotCollideWithPackage)
{
	ASSERT_NE(nullptr, _map.addTileScripted("tile-packagetarget-rock-01-idle", 2.0f, 12.0f));
	addGroundRow(8.0f, 3, 10);
	CaveMapTile* cave = static_cast<CaveMapTile*>(_map.addTileScripted("tile-cave-01", 4.0f, 7.0f));
	ASSERT_NE(nullptr, cave);
	Package* package = addPackage(6.0f, 7.2f);
	package->setLinearVelocity(PhysicsVec2_zero);
	NPCPackage* npc = _map.spawnPackageNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN);
	ASSERT_NE(nullptr, npc);
	EXPECT_FALSE(npc->shouldCollide(package));
	EXPECT_FALSE(package->shouldCollide(npc));
}

TEST_F(PhysicsTest, PlayerCollectsPackageFromAbove)
{
	ASSERT_NE(nullptr, _map.addTileScripted("tile-packagetarget-rock-01-idle", 2.0f, 12.0f));
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	player->setLinearVelocity(PhysicsVec2_zero);
	Package* package = addPackage(8.0f, 6.6f);
	package->setLinearVelocity(PhysicsVec2_zero);
	tick(20);
	EXPECT_TRUE(package->isCollected() || !player->isFree());
}

}
