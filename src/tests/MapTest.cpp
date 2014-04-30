#include "MapTest.h"
#include "caveexpress/server/map/Map.h"
#include "engine/common/ConfigManager.h"

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

class MapTest: public MapSuite {
protected:
	Map _map;

	virtual void SetUp() override {
		MapSuite::SetUp();
		_map.init(&_testFrontend, _serviceProvider);
	}
};

TEST_F(MapTest, testPlatform) {
	ASSERT_TRUE(_map.load("test-platform")) << "Could not load the map";
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
}

TEST_F(MapTest, testPlatformOneBigPlatform) {
	ASSERT_TRUE(_map.load("test-platform-big")) << "Could not load the map";

	GroundVisitor v(_map, 0, 2, 0, _map.getMapWidth() - 1);
	_map.visitEntities(&v);
}

TEST_F(MapTest, testMultipleLoad) {
	for (int i = 0; i < 100; ++i) {
		ASSERT_TRUE(_map.load("ice-01")) << "Could not load the map ice-01";
		Player* player = new Player(_map, 1);
		player->setLives(3);
		_map.initPlayer(player);
		_map.spawnPlayer(player);
		_map.startMap();
		ASSERT_TRUE(_map.load("ice-01")) << "Could not load the map ice-01";
	}
}
