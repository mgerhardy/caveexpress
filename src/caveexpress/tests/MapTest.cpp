#include "tests/TestShared.h"
#include "caveexpress/main/CaveExpress.h"
#include "caveexpress/shared/CaveExpressMapFailedReasons.h"
#include "common/ConfigManager.h"
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
			ASSERT_TRUE(--ticksLeft > 0) << mapName << " needs more ticks than the expected " << expectedTicks;
		}
		ASSERT_EQ(crashReason, map->getFailReason(player)) << mapName << ": unexpected crash reason";
		_game.shutdown();
	}

	void testSuccess (const std::string& mapName, MapTickCallback& callback, int ticksLeft = 10000) {
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
	ConfigVarPtr v = Config.getConfigVar("damagethreshold");
	v->setValue("1.0");
	testCrash("test-crash-hitpoints", MapFailedReasons::FAILED_HITPOINTS);
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
