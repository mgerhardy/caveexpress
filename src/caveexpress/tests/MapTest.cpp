#include "tests/TestShared.h"
#include "caveexpress/main/CaveExpress.h"
#include "caveexpress/shared/CaveExpressMapFailedReasons.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "client/entities/ClientEntityFactory.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/npcs/NPCFriendly.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/Platform.h"
#include "caveexpress/shared/constants/NPCState.h"
#include "caveexpress/server/entities/PackageTarget.h"
#include "common/ConfigManager.h"
#include "common/LobbyPlayers.h"
#include "common/Direction.h"
#include "common/EntityType.h"
#include "network/INetwork.h"
#include <algorithm>

namespace caveexpress {

class GroundVisitor: public IEntityVisitor {
private:
	const Map& _map;
	int _startGridX;
	int _gridY;
	int _expectedStart;
	int _expectedEnd;
public:
	GroundVisitor(const Map& map, int startGridX, int gridY, int expectedStart, int expectedEnd) :
			_map(map), _startGridX(startGridX), _gridY(gridY), _expectedStart(expectedStart), _expectedEnd(expectedEnd) {
	}
	// IEntityVisitor
	bool visitEntity(IEntity *entity) override {
		if (entity->isGround()) {
			const MapTile *mapTile = static_cast<const MapTile*>(entity);
			const int mapGridX = (int)(mapTile->getGridX() + EPSILON);
			if (mapGridX >= _startGridX && mapGridX <= _expectedEnd && fequals(mapTile->getGridY(), _gridY)) {
				int gridStart = -1;
				int gridEnd = -1;
				const int y = mapTile->getGridY() - 1.0f + EPSILON;
				_map.getPlatformDimensions(mapTile->getGridX(), y, &gridStart, &gridEnd);
				EXPECT_EQ(_expectedStart, gridStart) << "Invalid platform grid start found: "
						<< mapTile->getSpriteID() << ", map: " << _map.getName()
						<< ", expectedstart: " << _expectedStart << ", expectedend: " << _expectedEnd
						<< ", x: " << _startGridX << ", y: " << _gridY;
				EXPECT_EQ(_expectedEnd, gridEnd) << "Invalid platform grid end found: "
						<< mapTile->getSpriteID() << ", map: " << _map.getName()
						<< ", expectedstart: " << _expectedStart << ", expectedend: " << _expectedEnd
						<< ", x: " << _startGridX << ", y: " << _gridY;
			}
		}
		return IEntityVisitor::visitEntity(entity);
	}
};

class MapTest: public AbstractTest {
protected:
	CaveExpress _game;
	Map _map;

	class MapTickCallback {
	public:
		virtual ~MapTickCallback() {}

		virtual void exec(Map* map, Player* player) = 0;

		void operator()(Map* map, Player* player) {
			exec(map, player);
		}
	};

	void testCrash (const std::string& mapName, const MapFailedReason& crashReason, int ticksLeft = 10000) {
		Config.getConfigVar(GOD_MODE)->setValue("false");
		ASSERT_TRUE(_game.mapLoad(mapName)) << "Could not load the map " << mapName;
		Map* map = &_game.getMap();
		Player* player = new Player(*map, 1);
		player->setLives(3);
		ASSERT_TRUE(map->initPlayer(player)) << mapName << ": could not init player";
		map->startMap();
		ASSERT_TRUE(map->isActive()) << mapName << " is not active";
		const int expectedTicks = ticksLeft;
		while (!player->isCrashed() && !map->isFailed()) {
			_game.update(1);
			ASSERT_TRUE(--ticksLeft > 0) << mapName << " needs more ticks than the expected " << expectedTicks << " - player still has " << player->getHitpoints() << " hitpoints left";;
		}
		ASSERT_EQ(crashReason, map->getFailReason(player)) << mapName << ": unexpected crash reason - player still has " << player->getHitpoints() << " hitpoints left";
		_game.shutdown();
	}

	void testSuccess (const std::string& mapName, MapTickCallback& callback, int ticksLeft = 10000) {
		Config.getConfigVar(MAX_HITPOINTS)->setValue("100");
		Config.getConfigVar(GOD_MODE)->setValue("false");
		ASSERT_TRUE(_game.mapLoad(mapName)) << "Could not load the map " << mapName;
		Map* map = &_game.getMap();
		Player* player = new Player(*map, 1);
		player->setLives(3);
		ASSERT_TRUE(map->initPlayer(player)) << mapName << ": could not init player";
		map->startMap();
		ASSERT_TRUE(map->isActive()) << mapName << " is not active";
		const int expectedTicks = ticksLeft;
		while (!map->isFailed() && !map->isDone()) {
			_game.update(1);
			callback(map, player);
			ASSERT_TRUE(--ticksLeft > 0) << mapName << " needs more ticks than the expected " << expectedTicks;
		}
		ASSERT_FALSE(map->isFailed()) << mapName << ": failed - but we didn't expect that";
		ASSERT_TRUE(map->isDone()) << mapName << ": should have been done";
		_game.shutdown();
	}

	virtual void SetUp() override {
		AbstractTest::SetUp();
		_serviceProvider.getNetwork().openServer(12345, nullptr);
		_map.init(&_testFrontend, _serviceProvider);
		_game.init(&_testFrontend, _serviceProvider);
		TextureDefinition t("small");
		SpriteDefinition::get().init(t);
		// Manual Start in tests: do not auto-start when the second player joins.
		Config.getConfigVar("maxplayers", "2")->setValue(MAX_CLIENTS);
	}
};

TEST_F(MapTest, testIntroMoviePackageLoadsAndFinishes) {
	ASSERT_TRUE(_map.load("intro-movie-package")) << "Could not load intro-movie-package";
	ASSERT_EQ(1, _map.getCaveCount());
	CaveMapTile* cave = _map.getCave(0);
	ASSERT_NE(nullptr, cave);
	// Cave must sit on a free flyable cell (not buried under a multi-cell solid).
	ASSERT_EQ(1, static_cast<int>(cave->getGridX() + EPSILON));
	ASSERT_EQ(4, static_cast<int>(cave->getGridY() + EPSILON));
	ASSERT_FALSE(_map.isInputEnabled()); // onMapLoaded disables input

	Player* player = new Player(_map, 1);
	player->setLives(3);
	ASSERT_TRUE(_map.initPlayer(player));
	_map.startMap();
	ASSERT_TRUE(_map.isActive());
	ASSERT_FALSE(_map.isInputEnabled());

	// Drive through waste/idea/dust/reveal, boarding, the dumper, and rescue.
	int seenPackages = 0;
	for (int i = 0; i < 4000; ++i) {
		_map.update(16);
		seenPackages = std::max(seenPackages, _map.countPackages() + _map.getDeliveredPackageCount());
	}

	// Fully scripted cutscene: input stays locked; packed garbage is staged after the build.
	ASSERT_FALSE(_map.isInputEnabled()) << "intro cutscene must never enable player input";
	ASSERT_GT(seenPackages, 0) << "script should spawn packages";

	_map.forceComplete();
	ASSERT_TRUE(_map.isDone());
}

TEST_F(MapTest, testScriptWaterAndCaveApis) {
	ASSERT_TRUE(_map.load("intro-movie-package")) << "Could not load intro-movie-package";
	CaveMapTile* cave = _map.getCave(0);
	ASSERT_NE(nullptr, cave);
	const bool lightBefore = cave->getLightState();
	cave->setLightState(!lightBefore);
	ASSERT_EQ(!lightBefore, cave->getLightState());
	cave->setNextSpawn(0);
	cave->setRespawnPossible(false, EntityType::NONE);

	_map.setWaterHeight(2.0f);
	// getWaterHeight returns the water body Y-derived height; just ensure the call is safe
	(void)_map.getWaterHeight();
}

TEST_F(MapTest, testScriptClientInputAndTileReplace) {
	ASSERT_TRUE(_map.load("intro-movie-package")) << "Could not load intro-movie-package";
	ASSERT_FALSE(_map.isInputEnabled());

	_map.noteClientDirectionPressed(DIRECTION_LEFT);
	ASSERT_TRUE(_map.isScriptClientDirectionPressed(DIRECTION_LEFT));
	ASSERT_FALSE(_map.isScriptClientSkipPressed()) << "fly keys must not latch cinematic skip";
	_map.noteClientDirectionReleased(DIRECTION_LEFT);
	ASSERT_FALSE(_map.isScriptClientDirectionPressed(DIRECTION_LEFT));

	_map.noteClientAction();
	ASSERT_TRUE(_map.isScriptClientActionPressed());
	ASSERT_TRUE(_map.isScriptClientSkipPressed());

	_map.clearScriptClientInput();
	_map.noteClientSkip();
	ASSERT_TRUE(_map.isScriptClientSkipPressed());

	// Enabling input clears latched cinematic skip/action so play phase is clean.
	_map.setInputEnabled(true);
	ASSERT_FALSE(_map.isScriptClientSkipPressed());
	ASSERT_FALSE(_map.isScriptClientActionPressed());

	_map.setInputEnabled(false);
	const int removed = _map.removeTileAtScripted(3, 5);
	ASSERT_GE(removed, 1);
	MapTile* replaced = _map.replaceTileScripted("tile-rock-02", 3, 5);
	ASSERT_NE(nullptr, replaced);
	_map.rebuildPlatforms();
}

TEST_F(MapTest, testScriptFriendlyNpcWithoutTargetCave) {
	ASSERT_TRUE(_map.load("intro-movie-package")) << "Could not load intro-movie-package";
	CaveMapTile* cave = _map.getCave(0);
	ASSERT_NE(nullptr, cave);
	// Single-cave map: normal createFriendlyNPC would fail without a destination.
	NPCFriendly* npc = _map.spawnFriendlyNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN, true);
	ASSERT_NE(nullptr, npc) << "scripted friendly spawn should work without a target cave";
	ASSERT_EQ(nullptr, npc->getTargetCave());
	npc->setTargetCave(cave);
	ASSERT_EQ(cave, npc->getTargetCave());
}

TEST_F(MapTest, testScriptPackageNpcSpawnAndPhysicsBindings) {
	ASSERT_TRUE(_map.load("intro-movie-package")) << "Could not load intro-movie-package";
	CaveMapTile* cave = _map.getCave(0);
	ASSERT_NE(nullptr, cave);
	NPCPackage* npc = _map.spawnPackageNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN);
	ASSERT_NE(nullptr, npc) << "scripted package NPC should spawn for the intro cave";
	ASSERT_EQ(npc, cave->getNPC()) << "scripted spawn must register NPC on the cave for moveBackIntoCave";

	Player* player = new Player(_map, 1);
	player->setLives(3);
	ASSERT_TRUE(_map.initPlayer(player));
	_map.startMap();
	ASSERT_TRUE(_map.isActive());

	const PhysicsVec2 before = player->getPos();
	player->setGravityScale(0.0f);
	player->setLinearVelocity(PhysicsVec2(-1.5f, -0.5f));
	player->applyLinearImpulse(PhysicsVec2(0.1f, 0.0f));
	for (int i = 0; i < 10; ++i)
		_map.update(16);
	const PhysicsVec2 after = player->getPos();
	ASSERT_NE(before.x, after.x) << "scripted velocity should move the player without input";
	player->setGravityScale(1.0f);
	player->setLinearVelocity(PhysicsVec2_zero);
}

TEST_F(MapTest, testScriptPackageNpcReturnsIntoCave) {
	ASSERT_TRUE(_map.load("intro-movie-package")) << "Could not load intro-movie-package";
	CaveMapTile* cave = _map.getCave(0);
	ASSERT_NE(nullptr, cave);
	cave->setRespawnPossible(false, EntityType::NONE);

	NPCPackage* npc = _map.spawnPackageNPCScripted(cave, EntityTypes::NPC_FRIENDLY_MAN);
	ASSERT_NE(nullptr, npc);
	ASSERT_EQ(npc, cave->getNPC());
	const uint16_t npcId = npc->getID();

	Player* player = new Player(_map, 1);
	player->setLives(3);
	ASSERT_TRUE(_map.initPlayer(player));
	_map.startMap();

	// Walk out, dump, walk home — then the cave must take the NPC back and remove it.
	for (int i = 0; i < 30; ++i)
		_map.update(16);
	Package* pkg = npc->leavePackage();
	ASSERT_NE(nullptr, pkg);

	bool removed = false;
	for (int i = 0; i < 600; ++i) {
		_map.update(16);
		if (_map.findEntity(npcId) == nullptr) {
			removed = true;
			break;
		}
	}
	ASSERT_TRUE(removed) << "package NPC should disappear after returning to the cave";
	ASSERT_EQ(nullptr, cave->getNPC());
	ASSERT_FALSE(cave->shouldSpawnNPC());
}

TEST_F(MapTest, testScriptPackageDeliveryApis) {
	ASSERT_TRUE(_map.load("intro-movie-package")) << "Could not load intro-movie-package";
	PackageTarget* target = _map.getPackageTarget();
	ASSERT_NE(nullptr, target);
	ASSERT_EQ(3, _map.getPackageDeliveryGoal());
	ASSERT_EQ(0, _map.getDeliveredPackageCount());
	ASSERT_EQ(0, _map.getCollectedPackageCount());
	ASSERT_FALSE(target->isPulling());
	ASSERT_EQ(nullptr, target->getPullingPackage());

	Player* player = new Player(_map, 1);
	player->setLives(3);
	ASSERT_TRUE(_map.initPlayer(player));
	_map.startMap();
	ASSERT_EQ(0, player->getCollectedPackageCount());
	ASSERT_EQ(nullptr, player->getCollectedPackage(0));
	ASSERT_EQ(player->getCollectedPackageCount(), _map.getCollectedPackageCount());
}

TEST_F(MapTest, testPlatform) {
	ASSERT_TRUE(_map.load("test-platform")) << "Could not load the map test-platform";
	{
		GroundVisitor v(_map, 0, 2, 0, _map.getMapWidth() - 1);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 0, 4, 0, 0);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 0, 6, 1, 4);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 0, 8, 0, 2);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 0, 9, 3, 5);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 0, 12, 0, 2);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 3, 12, 4, _map.getMapWidth() - 1);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 0, 14, 0, 0);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 2, 14, 2, 2);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 4, 14, 4, 4);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 0, 16, 1, 4);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 0, 18, 0, 3);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 5, 18, 5, 5);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 1, 22, 1, 1);
		_map.visitEntities(&v);
	}

	{
		GroundVisitor v(_map, 3, 22, 3, 4);
		_map.visitEntities(&v);
	}
	_map.shutdown();
}

TEST_F(MapTest, testPlatformOneBigPlatform) {
	ASSERT_TRUE(_map.load("test-platform-big")) << "Could not load the map test-platform-big";

	GroundVisitor v(_map, 0, 2, 0, _map.getMapWidth() - 1);
	_map.visitEntities(&v);
	_map.shutdown();
}

TEST_F(MapTest, testMultipleLoad) {
	for (int i = 0; i < 100; ++i) {
		ASSERT_TRUE(_map.load("ice-01")) << "Could not load the map ice-01";
		Player* player = new Player(_map, 1);
		player->setLives(3);
		ASSERT_TRUE(_map.initPlayer(player));
		ASSERT_FALSE(_map.initPlayer(player));
		ASSERT_FALSE(_map.initPlayer(player));
		_map.startMap();
		ASSERT_TRUE(_map.load("ice-01")) << "Could not load the map ice-01";
		_map.update(i);
		_map.shutdown();
	}
}

TEST_F(MapTest, testPlayerCrashFlyingPackage) {
	testCrash("test-crash-flying-package", MapFailedReasons::FAILED_NPC_FLYING);
}

TEST_F(MapTest, testPlayerCrashFishPackage) {
	testCrash("test-crash-fish-package", MapFailedReasons::FAILED_NPC_FISH);
}

TEST_F(MapTest, testPlayerCrashFishNothingCollected) {
	testCrash("test-crash-fish-nothing-collected", MapFailedReasons::FAILED_NPC_FISH);
}

TEST_F(MapTest, testPlayerCrashWalkingPackage) {
	testCrash("test-crash-walking-package", MapFailedReasons::FAILED_NPC_WALKING);
}

TEST_F(MapTest, testPlayerCrashWalkingStone) {
	testCrash("test-crash-walking-stone", MapFailedReasons::FAILED_NPC_WALKING);
}

TEST_F(MapTest, testPlayerCrashWater) {
	testCrash("test-crash-water", MapFailedReasons::FAILED_WATER_HEIGHT);
}

TEST_F(MapTest, testPlayerCrashSideScroll) {
	testCrash("test-crash-sidescroll", MapFailedReasons::FAILED_SIDESCROLL);
}

TEST_F(MapTest, testPlayerCrashHitpoints) {
	ConfigVarPtr v = Config.getConfigVar(DAMAGE_THRESHOLD);
	v->setValue("1.0");
	Config.getConfigVar(MAX_HITPOINTS)->setValue("1");
	testCrash("test-crash-hitpoints", MapFailedReasons::FAILED_HITPOINTS, 100000);
}

TEST_F(MapTest, testPlayerWinCondition) {
	class PackageCallback : public MapTickCallback {
	public:
		void exec(Map* map, Player* player) override {
			Log::info(LOG_GAMEIMPL, "%s", player->getName().c_str());
		}
	};
	PackageCallback c;
	testSuccess("test-win-package", c);
}

TEST_F(MapTest, testLetter01TaxiLandingAndKnockOff)
{
	ASSERT_TRUE(_map.load("letter-01")) << "Could not load letter-01";
	Player* player = new Player(_map, 1);
	player->setLives(3);
	ASSERT_TRUE(_map.initPlayer(player));
	_map.startMap();
	ASSERT_TRUE(_map.isActive());

	ASSERT_EQ(5, _map.getCaveCount());
	for (int i = 0; i < _map.getCaveCount(); ++i) {
		CaveMapTile* cave = _map.getCave(i);
		ASSERT_NE(nullptr, cave);
		Log::info(LOG_GAMEIMPL, "cave %i at %.1f,%.1f platform %i-%i size %.2fx%.2f",
				cave->getCaveNumber(), cave->getGridX(), cave->getGridY(),
				cave->getPlatformStartGridX(), cave->getPlatformEndGridX(),
				cave->getSize().x, cave->getSize().y);
	}

	CaveMapTile* cave5 = nullptr;
	for (int i = 0; i < _map.getCaveCount(); ++i) {
		if (_map.getCave(i)->getCaveNumber() == 5) {
			cave5 = _map.getCave(i);
			break;
		}
	}
	ASSERT_NE(nullptr, cave5);

	class PlatformVisitor: public IEntityVisitor {
	public:
		int platforms = 0;
		int withCave5 = 0;
		bool visitEntity (IEntity *entity) override
		{
			if (!entity->isPlatform())
				return false;
			++platforms;
			Platform* p = static_cast<Platform*>(entity);
			Log::info(LOG_GAMEIMPL, "platform at %.2f,%.2f size %.2fx%.2f cave=%i",
					p->getPos().x, p->getPos().y, p->getSize().x, p->getSize().y,
					p->getCave() ? p->getCave()->getCaveNumber() : -1);
			if (p->getCave() && p->getCave()->getCaveNumber() == 5)
				++withCave5;
			return false;
		}
	};
	PlatformVisitor platforms;
	_map.visitEntities(&platforms);
	EXPECT_GT(platforms.withCave5, 0) << "cave 5 should own a platform body";

	// Sit on the visible ledge under cave 5 (below the thin platform sensor).
	player->setGravityScale(0.0f);
	player->setLinearVelocity(PhysicsVec2_zero);
	PhysicsVec2 ledgePos;
	bool foundLedge = false;
	const float xs[] = { 12.5f, 13.2f, 13.5f, 12.8f };
	const float ys[] = { 5.7f, 5.9f, 6.1f, 6.3f, 6.5f, 5.5f };
	for (float x : xs) {
		for (float y : ys) {
			player->setPos(PhysicsVec2(x, y));
			if (player->isCloseOverSolid()) {
				ledgePos = PhysicsVec2(x, y);
				foundLedge = true;
				break;
			}
		}
		if (foundLedge)
			break;
	}
	ASSERT_TRUE(foundLedge) << "could not find a solid-over pose on cave 5 platform "
			<< cave5->getPlatformStartGridX() << "-" << cave5->getPlatformEndGridX();
	player->setPos(ledgePos);
	ASSERT_TRUE(player->isLandedOn(cave5)) << "spatial landing should accept cave 5 ground at "
			<< ledgePos.x << "," << ledgePos.y;

	CaveMapTile* src = _map.getCave(0);
	ASSERT_NE(cave5, src);
	NPCFriendly* npc = _map.spawnFriendlyNPCScripted(src, EntityTypes::NPC_FRIENDLY_MAN, false);
	ASSERT_NE(nullptr, npc);
	npc->setTargetCave(cave5);
	player->setCollectedNPC(npc);
	npc->setState(NPCState::NPC_COLLECTED);
	ASSERT_TRUE(player->isTransfering(npc));
	ASSERT_TRUE(npc->isCollected());
	// Real collect path hides the passenger (no physics) but must keep it updating.
	ASSERT_TRUE(_map.removeNPCFromWorld(npc));
	ASSERT_FALSE(npc->isRemove());
	player->setPos(ledgePos);
	player->setLinearVelocity(PhysicsVec2_zero);
	ASSERT_TRUE(player->isLandedOn(cave5)) << "still landed on cave 5 at " << ledgePos.x << "," << ledgePos.y;
	npc->update(16);
	EXPECT_TRUE(npc->isArrived() || !player->isTransfering(npc))
			<< "direct update after hide should drop off; collected=" << npc->isCollected()
			<< " pos=" << player->getPos().x << "," << player->getPos().y;

	// Soft-touch an idle NPC must not disable later knock-off collisions.
	CaveMapTile* cave1 = _map.getCave(0);
	INPCCave* caveNpc = cave1->getNPC();
	NPCFriendly* idle = nullptr;
	if (caveNpc != nullptr && !caveNpc->isDeliverPackage())
		idle = static_cast<NPCFriendly*>(caveNpc);
	if (idle == nullptr)
		idle = _map.spawnFriendlyNPCScripted(cave1, EntityTypes::NPC_FRIENDLY_WOMAN, false);
	ASSERT_NE(nullptr, idle);
	idle->setState(NPCState::NPC_IDLE);
	player->setCollectedNPC(nullptr);
	player->setPos(PhysicsVec2(10.5f, 2.0f));
	player->setLinearVelocity(PhysicsVec2(0.4f, 0.0f));
	ASSERT_FALSE(player->isLanded());
	idle->onContact(PhysicsContact(), player);
	EXPECT_TRUE(idle->shouldCollide(player))
			<< "a slow airborne bump must not make the idle NPC ignore the player";
}

TEST_F(MapTest, testLobbyPlayerListMarksHost)
{
	ASSERT_TRUE(_map.load("ice-01")) << "Could not load ice-01";
	Player* host = new Player(_map, 1);
	host->setLives(3);
	host->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	guest->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guest));
	EXPECT_TRUE(_map.isReadyToStart());
	EXPECT_EQ(1, _map.getHostClientId());
	const std::vector<std::string> names = _map.getLobbyPlayerNames();
	ASSERT_EQ(2u, names.size());
	EXPECT_EQ(std::string("Alice") + " (host)", names[0]);
	EXPECT_EQ("Bob", names[1]);
	_map.startMap();
	ASSERT_TRUE(_map.isActive());
	const std::vector<std::string> afterStart = _map.getLobbyPlayerNames();
	ASSERT_EQ(2u, afterStart.size());
	EXPECT_EQ(std::string("Alice") + " (host)", afterStart[0]);
	_map.shutdown();
}

TEST_F(MapTest, testReturnToLobbyAllowsSecondStart)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.isMultiplayerSession());
	ASSERT_TRUE(_map.load("ice-01")) << "Could not load ice-01";
	Player* host = new Player(_map, 1);
	host->setLives(3);
	host->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	guest->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guest));
	_map.startMap();
	ASSERT_EQ(2u, _map.getPlayers().size());
	ASSERT_TRUE(_map.isActive());
	EXPECT_EQ(1, _map.getHostClientId());

	ASSERT_TRUE(_map.returnToLobby());
	EXPECT_TRUE(_map.getPlayers().empty()) << "match entities are gone until the next start";
	EXPECT_EQ(1, _map.getHostClientId()) << "host stays the first client";
	EXPECT_FALSE(_map.isActive());
	EXPECT_FALSE(_map.isMatchStarted());
	EXPECT_FALSE(_map.isFailed()) << "the lobby must not look like a failed match";
	_map.restart(1000);
	EXPECT_FALSE(_map.isRestartInitialized()) << "lobby must not schedule a restart that would CloseMap";

	Player* hostAgain = new Player(_map, 1);
	hostAgain->setLives(3);
	hostAgain->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(hostAgain));
	Player* guestAgain = new Player(_map, 2);
	guestAgain->setLives(3);
	guestAgain->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guestAgain));
	EXPECT_TRUE(_map.isReadyToStart());
	const std::vector<std::string> names = _map.getLobbyPlayerNames();
	ASSERT_EQ(2u, names.size());
	EXPECT_EQ(std::string("Alice") + " (host)", names[0]);
	EXPECT_EQ("Bob", names[1]);
	_map.startMap();
	ASSERT_EQ(2u, _map.getPlayers().size());
	ASSERT_TRUE(_map.isActive());
	_map.shutdown();
}

TEST_F(MapTest, testSinglePlayerDoesNotReturnToLobby)
{
	ASSERT_FALSE(_map.isMultiplayerSession());
	ASSERT_TRUE(_map.load("ice-01"));
	Player* player = new Player(_map, 1);
	player->setLives(3);
	ASSERT_TRUE(_map.initPlayer(player));
	_map.startMap();
	ASSERT_TRUE(_map.isActive());
	EXPECT_FALSE(_map.returnToLobby());
	EXPECT_EQ(1u, _map.getPlayers().size()) << "single-player stay on the running map";
	EXPECT_TRUE(_map.isActive());
	_map.shutdown();
}

TEST_F(MapTest, testSinglePlayerStartsWithoutLobbyReadyCheck)
{
	ASSERT_TRUE(_map.load("ice-01")) << "Could not load ice-01";
	Player* player = new Player(_map, 1);
	player->setLives(3);
	player->setName("Solo");
	ASSERT_TRUE(_map.initPlayer(player));
	EXPECT_FALSE(_map.isReadyToStart()) << "one waiting player is not enough for the MP start handler";
	EXPECT_FALSE(_map.isMatchStarted());
	EXPECT_FALSE(_map.isFailed()) << "waiting to start is not a fail";
	ASSERT_EQ(1u, _map.getLobbyPlayerNames().size());
	EXPECT_EQ(std::string("Solo") + " (host)", _map.getLobbyPlayerNames()[0]);
	// Single player calls startMap() directly (CMD_START), not StartMapHandler.
	_map.startMap();
	ASSERT_TRUE(_map.isActive());
	ASSERT_TRUE(_map.isMatchStarted());
	ASSERT_EQ(1u, _map.getPlayers().size());
	EXPECT_FALSE(_map.isFailed());
	_map.shutdown();
}

TEST_F(MapTest, testPackageCavesKeepRespawnUntilQuota)
{
	ASSERT_TRUE(_map.load("rock-01")) << "Could not load rock-01";
	ASSERT_EQ(4, _map.getPackageCount());
	ASSERT_EQ(3, _map.getCaveCount());

	Player* player = new Player(_map, 1);
	player->setLives(3);
	ASSERT_TRUE(_map.initPlayer(player));
	_map.startMap();
	ASSERT_TRUE(_map.isActive());

	for (int i = 0; i < _map.getCaveCount(); ++i) {
		CaveMapTile* cave = _map.getCave(i);
		ASSERT_NE(nullptr, cave);
		cave->spawnNPC(true);
		ASSERT_NE(nullptr, cave->getNPC()) << "cave " << cave->getCaveNumber() << " should spawn a package NPC";
		EXPECT_TRUE(cave->isRespawnPossible()) << "cave " << cave->getCaveNumber()
				<< " must keep respawning while packagetransfercount is unmet";
	}
}

TEST_F(MapTest, testJoinDuringLobbySpawnsAsPlayer)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.isMultiplayerSession());
	ASSERT_TRUE(_map.load("ice-01")) << "Could not load ice-01";
	Player* host = new Player(_map, 1);
	host->setLives(3);
	host->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	guest->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guest));
	Player* lateLobby = new Player(_map, 3);
	lateLobby->setLives(3);
	lateLobby->setName("Carol");
	ASSERT_TRUE(_map.initPlayer(lateLobby));
	EXPECT_FALSE(_map.isMatchStarted());
	EXPECT_TRUE(_map.getSpectators().empty());
	EXPECT_EQ(3, _map.getConnectedPlayers());
	_map.startMap();
	ASSERT_EQ(3u, _map.getPlayers().size());
	EXPECT_TRUE(_map.getSpectators().empty());
	EXPECT_TRUE(_map.isMatchStarted());
	_map.shutdown();
}

TEST_F(MapTest, testJoinAfterStartIsSpectator)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.isMultiplayerSession());
	ASSERT_TRUE(_map.load("ice-01")) << "Could not load ice-01";
	Player* host = new Player(_map, 1);
	host->setLives(3);
	host->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	guest->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guest));
	_map.startMap();
	ASSERT_EQ(2u, _map.getPlayers().size());
	ASSERT_TRUE(_map.isMatchStarted());
	EXPECT_EQ(2, _map.countLivingPlayers());

	Player* late = new Player(_map, 3);
	late->setLives(3);
	late->setName("Carol");
	ASSERT_TRUE(_map.initPlayer(late));
	EXPECT_EQ(2u, _map.getPlayers().size()) << "late joiner must not spawn a ship";
	ASSERT_EQ(1u, _map.getSpectators().size());
	EXPECT_EQ(late, _map.getSpectators().front());
	EXPECT_TRUE(late->isSpectator());
	EXPECT_FALSE(late->isLive());
	EXPECT_FALSE(late->acceptsControlInput());
	EXPECT_EQ(2, _map.countLivingPlayers());
	EXPECT_EQ(3, _map.getConnectedPlayers());
	EXPECT_EQ(late, _map.getPlayer(3));
	EXPECT_FALSE(_map.isFailed());
	const std::vector<std::string> names = _map.getLobbyPlayerNames();
	ASSERT_EQ(3u, names.size());
	EXPECT_EQ(std::string("Carol") + lobby::SPECTATING_SUFFIX, names[2]);

	int platforms = 0;
	class ServerOnlyCount: public IEntityVisitor {
	public:
		int* platforms;
		explicit ServerOnlyCount (int* p) : platforms(p) {}
		bool visitEntity (IEntity *entity) override
		{
			if (entity->isPlatform()) {
				++*platforms;
				EXPECT_TRUE(entity->isServerOnly()) << entity->getType().name;
			}
			if (entity->isServerOnly()) {
				EXPECT_EQ(nullptr, ClientEntityRegistry::get(entity->getType(), entity->getID()))
					<< "late-join snapshot must not AddEntity " << entity->getType().name;
			}
			return false;
		}
	};
	ServerOnlyCount counter(&platforms);
	_map.visitEntities(&counter);
	EXPECT_GT(platforms, 0) << "ice-01 must have landing platforms that used to crash spectators";
	_map.shutdown();
}

TEST_F(MapTest, testSpectatorReturnsToLobbyAsPlayer)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.load("ice-01"));
	Player* host = new Player(_map, 1);
	host->setLives(3);
	host->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	guest->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guest));
	_map.startMap();
	Player* late = new Player(_map, 3);
	late->setLives(3);
	late->setName("Carol");
	ASSERT_TRUE(_map.initPlayer(late));
	ASSERT_EQ(1u, _map.getSpectators().size());

	ASSERT_TRUE(_map.returnToLobby());
	EXPECT_TRUE(_map.getSpectators().empty());
	EXPECT_FALSE(_map.isMatchStarted());

	Player* hostAgain = new Player(_map, 1);
	hostAgain->setLives(3);
	hostAgain->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(hostAgain));
	Player* guestAgain = new Player(_map, 2);
	guestAgain->setLives(3);
	guestAgain->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guestAgain));
	Player* lateAgain = new Player(_map, 3);
	lateAgain->setLives(3);
	lateAgain->setName("Carol");
	ASSERT_TRUE(_map.initPlayer(lateAgain));
	EXPECT_TRUE(_map.getSpectators().empty());
	EXPECT_FALSE(lateAgain->isSpectator());
	_map.startMap();
	ASSERT_EQ(3u, _map.getPlayers().size());
	_map.shutdown();
}

TEST_F(MapTest, testLobbyAutoStartsWhenMaxPlayersJoin)
{
	_serviceProvider.updateNetwork(true);
	Config.getConfigVar("maxplayers")->setValue(2);
	ASSERT_EQ(2, _map.getMaxPlayers());
	ASSERT_TRUE(_map.load("ice-01"));
	Player* host = new Player(_map, 1);
	host->setLives(3);
	host->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(host));
	EXPECT_FALSE(_map.isMatchStarted());
	EXPECT_EQ(1, _map.getSessionPlayerCount());
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	guest->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guest));
	EXPECT_TRUE(_map.isMatchStarted()) << "second player must auto-start a max-2 lobby";
	ASSERT_EQ(2u, _map.getPlayers().size());
	EXPECT_TRUE(_map.getSpectators().empty());
	_map.startMap();
	EXPECT_EQ(2u, _map.getPlayers().size()) << "host Start after auto-start is a no-op";
	_map.shutdown();
}

TEST_F(MapTest, testHostCanForceStartBeforeLobbyIsFull)
{
	_serviceProvider.updateNetwork(true);
	Config.getConfigVar("maxplayers")->setValue(2);
	ASSERT_TRUE(_map.load("ice-01"));
	Player* host = new Player(_map, 1);
	host->setLives(3);
	host->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(host));
	EXPECT_FALSE(_map.isReadyToStart());
	EXPECT_FALSE(_map.isMatchStarted());
	_map.startMap();
	EXPECT_TRUE(_map.isMatchStarted());
	ASSERT_EQ(1u, _map.getPlayers().size());
	Player* late = new Player(_map, 2);
	late->setLives(3);
	late->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(late));
	EXPECT_TRUE(late->isSpectator()) << "after a forced start, further joins watch";
	EXPECT_EQ(1u, _map.getPlayers().size());
	_map.shutdown();
}

TEST_F(MapTest, testLobbyDoesNotAutoStartBeforeMaxPlayers)
{
	_serviceProvider.updateNetwork(true);
	Config.getConfigVar("maxplayers")->setValue(4);
	ASSERT_TRUE(_map.load("ice-01"));
	Player* host = new Player(_map, 1);
	host->setLives(3);
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	ASSERT_TRUE(_map.initPlayer(guest));
	EXPECT_FALSE(_map.isMatchStarted());
	EXPECT_TRUE(_map.isReadyToStart());
	EXPECT_EQ(2, _map.getSessionPlayerCount());
	EXPECT_EQ(4, _map.getMaxPlayers());
	_map.shutdown();
}

TEST_F(MapTest, testPlayerHudSnapshotDefaults)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.load("ice-01")) << "Could not load ice-01";
	Player* host = new Player(_map, 1);
	host->setLives(3);
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(2);
	ASSERT_TRUE(_map.initPlayer(guest));
	_map.startMap();
	ASSERT_EQ(2u, _map.getPlayers().size());
	EXPECT_EQ(0, host->getHudTargetCave());
	EXPECT_TRUE(host->getHudCollectedType().isNone());
	EXPECT_EQ(3, host->getLives());
	EXPECT_EQ(2, guest->getLives());
	_map.shutdown();
}

TEST_F(MapTest, testGuestLeaveDoesNotEndHostMatch)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.load("ice-01"));
	Player* host = new Player(_map, 1);
	host->setLives(3);
	host->setName("Alice");
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	guest->setName("Bob");
	ASSERT_TRUE(_map.initPlayer(guest));
	_map.startMap();
	ASSERT_EQ(2u, _map.getPlayers().size());
	ASSERT_TRUE(_map.isMatchStarted());
	ASSERT_TRUE(_map.isActive());

	_map.disconnect(2);
	EXPECT_EQ(1u, _map.getPlayers().size()) << "host ship stays after the guest leaves";
	EXPECT_EQ(1, _map.getConnectedPlayers());
	EXPECT_EQ(1, _map.countLivingPlayers());
	EXPECT_TRUE(_map.isMatchStarted());
	EXPECT_TRUE(_map.isActive()) << "host must be able to finish the map alone";
	EXPECT_EQ("ice-01", _map.getName()) << "leaving client must not reset the running map";
	EXPECT_FALSE(_map.isFailed());
	_map.shutdown();
}

TEST_F(MapTest, testGuestLeaveLobbyKeepsHostWaiting)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.load("ice-01"));
	Player* host = new Player(_map, 1);
	host->setLives(3);
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	ASSERT_TRUE(_map.initPlayer(guest));
	EXPECT_FALSE(_map.isMatchStarted());
	EXPECT_EQ(2, _map.getConnectedPlayers());
	_map.disconnect(2);
	EXPECT_EQ(1, _map.getConnectedPlayers());
	EXPECT_FALSE(_map.isMatchStarted());
	EXPECT_EQ("ice-01", _map.getName());
	EXPECT_FALSE(_map.isReadyToStart());
	_map.shutdown();
}

TEST_F(MapTest, testFailHoldDoesNotReloadUntilContinue)
{
	_serviceProvider.updateNetwork(true);
	ASSERT_TRUE(_map.load("ice-01"));
	Player* host = new Player(_map, 1);
	host->setLives(3);
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	ASSERT_TRUE(_map.initPlayer(guest));
	_map.startMap();
	for (int i = 0; i < 5; ++i)
		_map.update(20);
	host->setCrashed(CRASH_DAMAGE);
	guest->setCrashed(CRASH_DAMAGE);
	ASSERT_TRUE(_map.isFailed());
	_map.notifyClientsFailed();
	_map.holdForEndScreen();
	const uint32_t frozen = _map.getTime();
	const std::string name = _map.getName();
	for (int i = 0; i < 30; ++i)
		_map.update(20);
	EXPECT_EQ(frozen, _map.getTime()) << "the failed map must not tick during the death screen";
	EXPECT_EQ(name, _map.getName());
	EXPECT_TRUE(_map.isMatchStarted()) << "Continue has not re-entered the lobby yet";
	EXPECT_TRUE(_map.isEndScreenHold());
	ASSERT_TRUE(_map.returnToLobby());
	EXPECT_FALSE(_map.isEndScreenHold());
	EXPECT_FALSE(_map.isMatchStarted());
	EXPECT_TRUE(_map.getPlayers().empty());
	_map.shutdown();
}

TEST_F(MapTest, testReturnToLobbyDoesNotAutoStart)
{
	_serviceProvider.updateNetwork(true);
	Config.getConfigVar("maxplayers")->setValue("2");
	ASSERT_TRUE(_map.load("ice-01"));
	Player* host = new Player(_map, 1);
	host->setLives(3);
	ASSERT_TRUE(_map.initPlayer(host));
	Player* guest = new Player(_map, 2);
	guest->setLives(3);
	ASSERT_TRUE(_map.initPlayer(guest));
	_map.startMap();
	ASSERT_TRUE(_map.isMatchStarted());
	ASSERT_TRUE(_map.returnToLobby());
	Player* hostAgain = new Player(_map, 1);
	hostAgain->setLives(3);
	ASSERT_TRUE(_map.initPlayer(hostAgain));
	Player* guestAgain = new Player(_map, 2);
	guestAgain->setLives(3);
	ASSERT_TRUE(_map.initPlayer(guestAgain));
	EXPECT_FALSE(_map.isMatchStarted()) << "after a match the lobby must wait for host Start";
	EXPECT_EQ(2, _map.getSessionPlayerCount());
	_map.shutdown();
}

}
