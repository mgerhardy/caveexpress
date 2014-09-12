#include "tests/cavepacker/SokubanMapTest.h"
#include "cavepacker/server/map/Map.h"
#include "cavepacker/server/entities/Player.h"
#include "tests/NetworkTestListener.h"

class SokubanMapTest: public MapSuite {
protected:
	void testSingleMap(const std::string& mapName, bool solve = true) {
		NetworkTestListener listener;
		NetworkTestServerListener serverListener;
		ASSERT_TRUE(_serviceProvider.getNetwork().openServer(12345, &serverListener)) << "Failed to open the server";
		ASSERT_TRUE(_serviceProvider.getNetwork().openClient("localhost", 12345, &listener)) << "Failed to open the client";

		testMap(mapName, solve);

		ASSERT_EQ(0, listener._errorCount) << listener._lastError;
		ASSERT_NE(0, listener._count);
		ASSERT_EQ(0, serverListener._errorCount) << serverListener._lastError;
		ASSERT_NE(0, serverListener._count);

		_serviceProvider.getNetwork().closeClient();
		_serviceProvider.getNetwork().closeServer();
	}

	void testMap(const std::string& mapName, bool solve = true) {
		Map map;
		map.init(&_testFrontend, _serviceProvider);
		ASSERT_TRUE(map.load(mapName)) << "Failed to load map " << mapName;
		_serviceProvider.getNetwork().update(1);
		Player* player = new Player(map, 1);
		ASSERT_TRUE(map.initPlayer(player)) << "Failed to init the player for map " << mapName;
		_serviceProvider.getNetwork().update(1);
		map.startMap();
		_serviceProvider.getNetwork().update(1);
		if (!solve)
			return;
		const int steps = map.solve();
		ASSERT_TRUE(map.isAutoSolve()) << "Map " << mapName << " is not in autoSolve mode";
		ASSERT_TRUE(steps >= 1) << "No step in solution for map " << mapName;
		for (int s = 0; s < steps; ++s) {
			map.update(10000);
			_serviceProvider.getNetwork().update(1);
			ASSERT_TRUE(map.isAutoSolve()) << "Map " << mapName << " is no longer in autoSolve mode in step " << s;
		}
		EXPECT_TRUE(map.isDone()) << "Autosolve for map " << mapName << " did not lead to a done map";
		_serviceProvider.getNetwork().update(1);
	}

	void testMapRegion (int begin)
	{
		NetworkTestListener listener;
		NetworkTestServerListener serverListener;
		ASSERT_TRUE(_serviceProvider.getNetwork().openServer(12345, &serverListener)) << "Failed to open the server";
		ASSERT_TRUE(_serviceProvider.getNetwork().openClient("localhost", 12345, &listener)) << "Failed to open the client";

		for (int i = begin; i < begin + 10; ++i) {
			std::string mapName = String::format("%04i", i);
			testMap(mapName);
		}
		ASSERT_EQ(0, listener._errorCount) << listener._lastError;
		ASSERT_NE(0, listener._count);
		ASSERT_EQ(0, serverListener._errorCount) << serverListener._lastError;
		ASSERT_NE(0, serverListener._count);

		_serviceProvider.getNetwork().closeClient();
		_serviceProvider.getNetwork().closeServer();
	}
};

// we don't have solutions here
TEST_F(SokubanMapTest, testTutorials)
{
	NetworkTestListener listener;
	NetworkTestServerListener serverListener;
	ASSERT_TRUE(_serviceProvider.getNetwork().openServer(12345, &serverListener)) << "Failed to open the server";
	ASSERT_TRUE(_serviceProvider.getNetwork().openClient("localhost", 12345, &listener)) << "Failed to open the client";

	testMap("tutorial0001", false);
	testMap("tutorial0002", false);
	testMap("tutorial0003", false);

	ASSERT_EQ(0, listener._errorCount) << listener._lastError;
	ASSERT_NE(0, listener._count);
	ASSERT_EQ(0, serverListener._errorCount) << serverListener._lastError;
	ASSERT_NE(0, serverListener._count);

	_serviceProvider.getNetwork().closeClient();
	_serviceProvider.getNetwork().closeServer();
}

TEST_F(SokubanMapTest, test0001) { testSingleMap("0001"); }
TEST_F(SokubanMapTest, test0002) { testSingleMap("0002"); }
// this map segfaults - but only if no other map was loaded before - player id is 127 - if we add one or remove one tile, it works
TEST_F(SokubanMapTest, test0003) { testSingleMap("0003"); }
TEST_F(SokubanMapTest, test0004) { testSingleMap("0004"); }
TEST_F(SokubanMapTest, test0005) { testSingleMap("0005"); }
TEST_F(SokubanMapTest, test0006) { testSingleMap("0006"); }
TEST_F(SokubanMapTest, test0007) { testSingleMap("0007"); }
TEST_F(SokubanMapTest, test0008) { testSingleMap("0008"); }
TEST_F(SokubanMapTest, test0009) { testSingleMap("0009"); }
TEST_F(SokubanMapTest, test0010) { testSingleMap("0010"); }
TEST_F(SokubanMapTest, test0011) { testSingleMap("0011"); }
TEST_F(SokubanMapTest, test0012) { testSingleMap("0012"); }
TEST_F(SokubanMapTest, test0013) { testSingleMap("0013"); }
TEST_F(SokubanMapTest, test0014) { testSingleMap("0014"); }
TEST_F(SokubanMapTest, test0015) { testSingleMap("0015"); }
TEST_F(SokubanMapTest, test0016) { testSingleMap("0016"); }
TEST_F(SokubanMapTest, test0017) { testSingleMap("0017"); }
TEST_F(SokubanMapTest, test0018) { testSingleMap("0018"); }
TEST_F(SokubanMapTest, test0019) { testSingleMap("0019"); }
TEST_F(SokubanMapTest, test0020) { testSingleMap("0020"); }
TEST_F(SokubanMapTest, test0021) { testSingleMap("0021"); }
TEST_F(SokubanMapTest, test0022) { testSingleMap("0022"); }
TEST_F(SokubanMapTest, test0023) { testSingleMap("0023"); }
TEST_F(SokubanMapTest, test0024) { testSingleMap("0024"); }
TEST_F(SokubanMapTest, test0025) { testSingleMap("0025"); }
TEST_F(SokubanMapTest, test0026) { testSingleMap("0026"); }
TEST_F(SokubanMapTest, test0027) { testSingleMap("0027"); }
TEST_F(SokubanMapTest, test0028) { testSingleMap("0028"); }
TEST_F(SokubanMapTest, test0029) { testSingleMap("0029"); }
TEST_F(SokubanMapTest, test0030) { testSingleMap("0030"); }
TEST_F(SokubanMapTest, test0031) { testSingleMap("0031"); }
TEST_F(SokubanMapTest, test0032) { testSingleMap("0032"); }
TEST_F(SokubanMapTest, test0033) { testSingleMap("0033"); }
TEST_F(SokubanMapTest, test0034) { testSingleMap("0034"); }
TEST_F(SokubanMapTest, test0035) { testSingleMap("0035"); }
TEST_F(SokubanMapTest, test0036) { testSingleMap("0036"); }
TEST_F(SokubanMapTest, test0037) { testSingleMap("0037"); }
TEST_F(SokubanMapTest, test0038) { testSingleMap("0038"); }
TEST_F(SokubanMapTest, test0039) { testSingleMap("0039"); }
TEST_F(SokubanMapTest, test0040) { testSingleMap("0040"); }
TEST_F(SokubanMapTest, test0041) { testSingleMap("0041"); }
TEST_F(SokubanMapTest, test0042) { testSingleMap("0042"); }
TEST_F(SokubanMapTest, test0043) { testSingleMap("0043"); }
TEST_F(SokubanMapTest, test0044) { testSingleMap("0044"); }
TEST_F(SokubanMapTest, test0045) { testSingleMap("0045"); }
TEST_F(SokubanMapTest, test0046) { testSingleMap("0046"); }
TEST_F(SokubanMapTest, test0047) { testSingleMap("0047"); }
TEST_F(SokubanMapTest, test0048) { testSingleMap("0048"); }
TEST_F(SokubanMapTest, test0049) { testSingleMap("0049"); }
TEST_F(SokubanMapTest, test0050) { testSingleMap("0050"); }
TEST_F(SokubanMapTest, test0051) { testSingleMap("0051"); }
TEST_F(SokubanMapTest, test0052) { testSingleMap("0052"); }
// this map segfaults - but only if no other map was loaded before - if we add one or remove one tile, it works
TEST_F(SokubanMapTest, test0053) { testSingleMap("0053"); }
TEST_F(SokubanMapTest, test0054) { testSingleMap("0054"); }
// this map segfaults - but only if no other map was loaded before - if we add one or remove one tile, it works
TEST_F(SokubanMapTest, test0055) { testSingleMap("0055"); }
TEST_F(SokubanMapTest, test0056) { testSingleMap("0056"); }
TEST_F(SokubanMapTest, test0057) { testSingleMap("0057"); }
TEST_F(SokubanMapTest, test0058) { testSingleMap("0058"); }
TEST_F(SokubanMapTest, test0059) { testSingleMap("0059"); }
// this map segfaults - but only if no other map was loaded before - if we add one or remove one tile, it works
TEST_F(SokubanMapTest, test0060) { testSingleMap("0060"); }
TEST_F(SokubanMapTest, test0061) { testSingleMap("0061"); }
TEST_F(SokubanMapTest, test0062) { testSingleMap("0062"); }
TEST_F(SokubanMapTest, test0063) { testSingleMap("0063"); }
TEST_F(SokubanMapTest, test0064) { testSingleMap("0064"); }
TEST_F(SokubanMapTest, test0065) { testSingleMap("0065"); }
TEST_F(SokubanMapTest, test0066) { testSingleMap("0066"); }
TEST_F(SokubanMapTest, test0067) { testSingleMap("0067"); }
TEST_F(SokubanMapTest, test0068) { testSingleMap("0068"); }
TEST_F(SokubanMapTest, test0069) { testSingleMap("0069"); }
TEST_F(SokubanMapTest, test0070) { testSingleMap("0070"); }
TEST_F(SokubanMapTest, test0071) { testSingleMap("0071"); }
TEST_F(SokubanMapTest, test0072) { testSingleMap("0072"); }
TEST_F(SokubanMapTest, test0073) { testSingleMap("0073"); }
TEST_F(SokubanMapTest, test0074) { testSingleMap("0074"); }
TEST_F(SokubanMapTest, test0075) { testSingleMap("0075"); }
TEST_F(SokubanMapTest, test0076) { testSingleMap("0076"); }
TEST_F(SokubanMapTest, test0077) { testSingleMap("0077"); }
TEST_F(SokubanMapTest, test0078) { testSingleMap("0078"); }
TEST_F(SokubanMapTest, test0079) { testSingleMap("0079"); }
TEST_F(SokubanMapTest, test0080) { testSingleMap("0080"); }
TEST_F(SokubanMapTest, test0081) { testSingleMap("0081"); }
TEST_F(SokubanMapTest, test0082) { testSingleMap("0082"); }
TEST_F(SokubanMapTest, test0083) { testSingleMap("0083"); }
TEST_F(SokubanMapTest, test0084) { testSingleMap("0084"); }
TEST_F(SokubanMapTest, test0085) { testSingleMap("0085"); }
TEST_F(SokubanMapTest, test0086) { testSingleMap("0086"); }
TEST_F(SokubanMapTest, test0087) { testSingleMap("0087"); }
TEST_F(SokubanMapTest, test0088) { testSingleMap("0088"); }
TEST_F(SokubanMapTest, test0089) { testSingleMap("0089"); }
TEST_F(SokubanMapTest, test0090) { testSingleMap("0090"); }

TEST_F(SokubanMapTest, testMaps10)
{
	testMapRegion(1);
}

TEST_F(SokubanMapTest, testMaps20)
{
	testMapRegion(11);
}

TEST_F(SokubanMapTest, testMaps30)
{
	testMapRegion(21);
}

TEST_F(SokubanMapTest, testMaps40)
{
	testMapRegion(31);
}

TEST_F(SokubanMapTest, testMaps50)
{
	testMapRegion(41);
}

TEST_F(SokubanMapTest, testMaps60)
{
	testMapRegion(51);
}

TEST_F(SokubanMapTest, testMaps70)
{
	testMapRegion(61);
}

TEST_F(SokubanMapTest, testMaps80)
{
	testMapRegion(71);
}

TEST_F(SokubanMapTest, testMaps90)
{
	testMapRegion(81);
}
