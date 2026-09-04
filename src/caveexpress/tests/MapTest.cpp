#include "tests/TestShared.h"
#include "caveexpress/main/CaveExpress.h"
#include "caveexpress/shared/CaveExpressMapFailedReasons.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/npcs/NPCFriendly.h"
#include "caveexpress/server/entities/npcs/NPCPackage.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/PackageTarget.h"
#include "common/ConfigManager.h"
#include "common/Direction.h"
#include "common/EntityType.h"
#include "network/INetwork.h"

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

	// Drive through waste/idea/dust/reveal and boarding into scripted rescue.
	for (int i = 0; i < 2000; ++i)
		_map.update(16);

	// Fully scripted cutscene: input stays locked; packed garbage is staged after the build.
	ASSERT_FALSE(_map.isInputEnabled()) << "intro cutscene must never enable player input";
	ASSERT_GT(_map.countPackages(), 0) << "script should spawn packages";

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

}
