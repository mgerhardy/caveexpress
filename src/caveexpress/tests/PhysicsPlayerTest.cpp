#include "PhysicsTest.h"
#include "caveexpress/server/entities/Border.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/Gate.h"
#include "caveexpress/server/entities/Platform.h"
#include "caveexpress/server/entities/PressurePlate.h"

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

TEST_F(PhysicsTest, MultiplayerOneDeathDoesNotEndMatch)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.isMultiplayerSession());
	ASSERT_NE(nullptr, addGround(8.0f, 12.0f));
	Player* a = addPlayer(8.0f, 6.0f);
	Player* b = _map.spawnPlayerAt(10.0f, 6.0f, 2);
	ASSERT_NE(nullptr, a);
	ASSERT_NE(nullptr, b);
	ASSERT_EQ(2u, _map.getPlayers().size());
	EXPECT_EQ(2, _map.countLivingPlayers());
	// setCrashed is a no-op while entity time is still 0 (treated as invulnerable).
	tick(1);

	a->setCrashed(CRASH_DAMAGE);
	a->setLives(0);
	EXPECT_TRUE(a->isCrashed());
	EXPECT_TRUE(a->isDead());
	EXPECT_EQ(1, _map.countLivingPlayers());
	EXPECT_FALSE(_map.isFailed());

	EXPECT_EQ(1, _map.handleDeadPlayers());
	EXPECT_EQ(2u, _map.getPlayers().size()) << "dead player must stay connected in multiplayer";
	EXPECT_EQ(b, _map.getPlayer(2));
	EXPECT_TRUE(_map.isActive());
	EXPECT_FALSE(_map.isFailed());
}

TEST_F(PhysicsTest, CrashedPlayerIgnoresInputAndKeepsCrashedAnimation)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_NE(nullptr, addGround(8.0f, 12.0f));
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	tick(1);
	player->setCrashed(CRASH_DAMAGE);
	player->setLives(0);
	ASSERT_TRUE(player->isCrashed());
	ASSERT_TRUE(player->isDead());
	ASSERT_FALSE(player->acceptsControlInput());
	EXPECT_TRUE(player->getAnimationType() == Animations::ANIMATION_CRASHED);

	player->accelerate(DIRECTION_UP);
	EXPECT_TRUE(player->getAnimationType() == Animations::ANIMATION_CRASHED)
			<< "fly input must not revive a crashed ship";

	player->resetAcceleration(DIRECTION_UP);
	EXPECT_TRUE(player->getAnimationType() == Animations::ANIMATION_CRASHED)
			<< "releasing a key must not switch a wreck back to idle";

	player->setFingerAcceleration(10, -10);
	player->resetFingerAcceleration();
	EXPECT_TRUE(player->getAnimationType() == Animations::ANIMATION_CRASHED);
}

TEST_F(PhysicsTest, CrashedPlayerDropsAllPackages)
{
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	tick(1);
	Package* first = addPackage(6.0f, 6.0f);
	Package* second = addPackage(10.0f, 6.0f);
	ASSERT_NE(nullptr, first);
	ASSERT_NE(nullptr, second);
	ASSERT_TRUE(player->collect(first));
	first->setCollected(true, player);
	ASSERT_TRUE(player->collect(second));
	second->setCollected(true, player);
	ASSERT_EQ(2, player->getCollectedPackageCount());

	player->setCrashed(CRASH_DAMAGE);

	EXPECT_EQ(0, player->getCollectedPackageCount());
	EXPECT_FALSE(first->isCollected());
	EXPECT_FALSE(second->isCollected());
}

TEST_F(PhysicsTest, CrashedPlayerReleasesPassengerDying)
{
	CaveMapTile* cave = static_cast<CaveMapTile*>(_map.addTileScripted("tile-cave-01", 5.0f, 9.0f));
	ASSERT_NE(nullptr, cave);
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	tick(1);
	NPCFriendly* passenger = _map.spawnFriendlyNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN, false);
	ASSERT_NE(nullptr, passenger);
	player->setCollectedNPC(passenger);
	passenger->setState(NPCState::NPC_COLLECTED);
	ASSERT_TRUE(_map.removeNPCFromWorld(passenger));
	ASSERT_EQ(passenger, cave->getNPC());
	ASSERT_TRUE(player->isTransfering(passenger));
	ASSERT_TRUE(passenger->getBodies().empty());

	player->setCrashed(CRASH_DAMAGE);

	EXPECT_FALSE(player->isTransfering(passenger));
	EXPECT_TRUE(passenger->isDying());
	EXPECT_FALSE(passenger->isFalling());
	EXPECT_FALSE(passenger->isStruggle());
	EXPECT_FALSE(passenger->getBodies().empty());
	EXPECT_EQ(nullptr, cave->getNPC());
}

TEST_F(PhysicsTest, DyingPassengerIgnoresWorldGeometry)
{
	addGroundRow(10.0f, 3, 12);
	CaveMapTile* cave = static_cast<CaveMapTile*>(_map.addTileScripted("tile-cave-01", 5.0f, 9.0f));
	MapTile* lava = _map.addTileScripted("tile-lava-rock-left-01", 10.0f, 10.0f);
	MapTile* bridge = _map.addTileScripted("bridge-plank-01", 7.0f, 8.0f);
	PressurePlate* plate = static_cast<PressurePlate*>(_map.addTileScripted("tile-plate-01-idle", 12.0f, 10.0f));
	Gate* gate = static_cast<Gate*>(_map.addTileScripted("tile-gate-rock-01", 14.0f, 8.0f));
	ASSERT_NE(nullptr, cave);
	ASSERT_NE(nullptr, lava);
	ASSERT_NE(nullptr, bridge);
	ASSERT_NE(nullptr, plate);
	ASSERT_NE(nullptr, gate);
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	tick(1);
	NPCFriendly* passenger = _map.spawnFriendlyNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN, false);
	ASSERT_NE(nullptr, passenger);
	player->setCollectedNPC(passenger);
	player->setCrashed(CRASH_DAMAGE);
	ASSERT_TRUE(passenger->isDying());

	EXPECT_FALSE(lava->shouldCollide(passenger));
	EXPECT_FALSE(passenger->shouldCollide(lava));
	EXPECT_FALSE(bridge->shouldCollide(passenger));
	EXPECT_FALSE(passenger->shouldCollide(bridge));
	EXPECT_FALSE(plate->shouldCollide(passenger));
	EXPECT_FALSE(gate->shouldCollide(passenger));

	Platform* platform = findType<Platform>(EntityTypes::PLATFORM);
	ASSERT_NE(nullptr, platform);
	EXPECT_FALSE(platform->shouldCollide(passenger));
	EXPECT_FALSE(passenger->shouldCollide(platform));

	ASSERT_FALSE(passenger->getBodies().empty());
	ASSERT_FALSE(lava->getBodies().empty());
	ASSERT_FALSE(bridge->getBodies().empty());
	ASSERT_FALSE(plate->getBodies().empty());
	ASSERT_FALSE(gate->getBodies().empty());
	ASSERT_FALSE(platform->getBodies().empty());
	const PhysicsFixture npcFix = passenger->getBodies()[0].getFixtureList();
	EXPECT_FALSE(_map.filterCollide(npcFix, lava->getBodies()[0].getFixtureList()));
	EXPECT_FALSE(_map.filterCollide(npcFix, bridge->getBodies()[0].getFixtureList()));
	EXPECT_FALSE(_map.filterCollide(npcFix, plate->getBodies()[0].getFixtureList()));
	EXPECT_FALSE(_map.filterCollide(npcFix, gate->getBodies()[0].getFixtureList()));
	EXPECT_FALSE(_map.filterCollide(npcFix, platform->getBodies()[0].getFixtureList()));
}

TEST_F(PhysicsTest, DyingPassengerStaysDyingInWater)
{
	CaveMapTile* cave = static_cast<CaveMapTile*>(_map.addTileScripted("tile-cave-01", 5.0f, 9.0f));
	ASSERT_NE(nullptr, cave);
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	tick(1);
	NPCFriendly* passenger = _map.spawnFriendlyNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN, false);
	ASSERT_NE(nullptr, passenger);
	player->setCollectedNPC(passenger);
	player->setCrashed(CRASH_DAMAGE);
	ASSERT_TRUE(passenger->isDying());

	freeze(passenger, PhysicsVec2(8.0f, 14.5f));
	tick(20);
	EXPECT_TRUE(passenger->isDying());
	EXPECT_FALSE(passenger->isStruggle());
	EXPECT_FALSE(passenger->isSwimming());
	EXPECT_FALSE(passenger->isIdle());
}

TEST_F(PhysicsTest, DyingPassengerRemovedAtBottomBorder)
{
	CaveMapTile* cave = static_cast<CaveMapTile*>(_map.addTileScripted("tile-cave-01", 5.0f, 9.0f));
	ASSERT_NE(nullptr, cave);
	Player* player = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, player);
	tick(1);
	NPCFriendly* passenger = _map.spawnFriendlyNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN, false);
	ASSERT_NE(nullptr, passenger);
	player->setCollectedNPC(passenger);
	player->setCrashed(CRASH_DAMAGE);
	ASSERT_TRUE(passenger->isDying());

	Border bottom(BorderType::BOTTOM, _map);
	passenger->onContact(PhysicsContact(), &bottom);
	EXPECT_TRUE(passenger->isRemove());
}

TEST_F(PhysicsTest, CrashedPlayerKeepsWorldCollision)
{
	_serviceProvider.updateNetwork(true);
	MapTile* ground = addGround(8.0f, 12.0f);
	ASSERT_NE(nullptr, ground);
	Player* living = addPlayer(8.0f, 6.0f);
	Player* wreck = _map.spawnPlayerAt(10.0f, 6.0f, 2);
	ASSERT_NE(nullptr, living);
	ASSERT_NE(nullptr, wreck);
	tick(1);
	wreck->setCrashed(CRASH_DAMAGE);
	wreck->setLives(0);
	ASSERT_FALSE(wreck->isLive());
	ASSERT_TRUE(living->isLive());

	EXPECT_FALSE(wreck->shouldCollide(living));
	EXPECT_FALSE(living->shouldCollide(wreck));
	EXPECT_TRUE(wreck->shouldCollide(ground)) << "wrecks must still rest on tiles";
	EXPECT_TRUE(living->shouldCollide(ground));

	IEntity* water = findType<IEntity>(EntityTypes::WATER);
	ASSERT_NE(nullptr, water);
	EXPECT_TRUE(wreck->shouldCollide(water)) << "wrecks must still get buoyancy";
	EXPECT_TRUE(wreck->shouldApplyWind()) << "map wind still pushes wrecks";

	ASSERT_FALSE(wreck->getBodies().empty());
	ASSERT_FALSE(living->getBodies().empty());
	ASSERT_FALSE(ground->getBodies().empty());
	const PhysicsFixture wreckFix = wreck->getBodies()[0].getFixtureList();
	const PhysicsFixture livingFix = living->getBodies()[0].getFixtureList();
	const PhysicsFixture groundFix = ground->getBodies()[0].getFixtureList();
	EXPECT_FALSE(_map.filterCollide(wreckFix, livingFix));
	EXPECT_TRUE(_map.filterCollide(wreckFix, groundFix));
	EXPECT_TRUE(_map.filterCollide(livingFix, groundFix));
}

TEST_F(PhysicsTest, MultiplayerAllDeadFailsMap)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.isMultiplayerSession());
	ASSERT_NE(nullptr, addGround(8.0f, 12.0f));
	Player* a = addPlayer(8.0f, 6.0f);
	Player* b = _map.spawnPlayerAt(10.0f, 6.0f, 2);
	ASSERT_NE(nullptr, a);
	ASSERT_NE(nullptr, b);
	tick(1);
	a->setCrashed(CRASH_DAMAGE);
	a->setLives(0);
	b->setCrashed(CRASH_DAMAGE);
	b->setLives(0);
	EXPECT_EQ(0, _map.countLivingPlayers());
	EXPECT_TRUE(_map.isFailed());
	EXPECT_EQ(2, _map.handleDeadPlayers());
	EXPECT_EQ(2u, _map.getPlayers().size()) << "spectators stay until the match ends";
}

TEST_F(PhysicsTest, MultiplayerLobbyMarksFirstPlayerAsHost)
{
	_serviceProvider.updateNetwork(true);
	Player* a = addPlayer(8.0f, 6.0f);
	Player* b = _map.spawnPlayerAt(10.0f, 6.0f, 2);
	ASSERT_NE(nullptr, a);
	ASSERT_NE(nullptr, b);
	a->setName("A");
	b->setName("B");
	EXPECT_EQ(1, _map.getHostClientId());
	const std::vector<std::string> names = _map.getLobbyPlayerNames();
	ASSERT_EQ(2u, names.size());
	EXPECT_EQ(std::string("A") + " (host)", names[0]);
	EXPECT_EQ("B", names[1]);
}

TEST_F(PhysicsTest, SinglePlayerLobbyListStillWorks)
{
	ASSERT_FALSE(_map.isMultiplayerSession());
	Player* a = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, a);
	a->setName("Solo");
	const std::vector<std::string> names = _map.getLobbyPlayerNames();
	ASSERT_EQ(1u, names.size());
	EXPECT_EQ(std::string("Solo") + " (host)", names[0]);
	EXPECT_FALSE(_map.isFailed());
	EXPECT_EQ(1u, _map.getPlayers().size());
}

TEST_F(PhysicsTest, SinglePlayerDeathStillLeavesTheMap)
{
	ASSERT_FALSE(_map.isMultiplayerSession());
	Player* a = addPlayer(8.0f, 6.0f);
	ASSERT_NE(nullptr, a);
	tick(1);
	a->setCrashed(CRASH_DAMAGE);
	a->setLives(0);
	EXPECT_EQ(1, _map.handleDeadPlayers());
	EXPECT_TRUE(_map.getPlayers().empty());
}

}
