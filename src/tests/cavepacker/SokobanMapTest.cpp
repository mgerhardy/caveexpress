#include "tests/cavepacker/SokobanMapTest.h"
#include "cavepacker/server/map/Map.h"
#include "cavepacker/server/entities/Player.h"
#include "tests/NetworkTestListener.h"
#include "data.h"

class SokobanMapTest: public MapSuite {
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

	void testMapRegion (int begin, const std::string& prefix = "xsokoban")
	{
		NetworkTestListener listener;
		NetworkTestServerListener serverListener;
		ASSERT_TRUE(_serviceProvider.getNetwork().openServer(12345, &serverListener)) << "Failed to open the server";
		ASSERT_TRUE(_serviceProvider.getNetwork().openClient("localhost", 12345, &listener)) << "Failed to open the client";

		for (int i = begin; i < begin + 10; ++i) {
			std::string mapName = prefix + String::format("%04i", i);
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
TEST_F(SokobanMapTest, testTutorials)
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

TEST_F(SokobanMapTest, testXSokoban0001) { testSingleMap("xsokoban0001"); }
TEST_F(SokobanMapTest, testXSokoban0002) { testSingleMap("xsokoban0002"); }
// this map segfaults - but only if no other map was loaded before - player id is 127 - if we add one or remove one tile, it works
TEST_F(SokobanMapTest, testXSokoban0003) { testSingleMap("xsokoban0003"); }
TEST_F(SokobanMapTest, testXSokoban0004) { testSingleMap("xsokoban0004"); }
TEST_F(SokobanMapTest, testXSokoban0005) { testSingleMap("xsokoban0005"); }
TEST_F(SokobanMapTest, testXSokoban0006) { testSingleMap("xsokoban0006"); }
TEST_F(SokobanMapTest, testXSokoban0007) { testSingleMap("xsokoban0007"); }
TEST_F(SokobanMapTest, testXSokoban0008) { testSingleMap("xsokoban0008"); }
TEST_F(SokobanMapTest, testXSokoban0009) { testSingleMap("xsokoban0009"); }
TEST_F(SokobanMapTest, testXSokoban0010) { testSingleMap("xsokoban0010"); }
TEST_F(SokobanMapTest, testXSokoban0011) { testSingleMap("xsokoban0011"); }
TEST_F(SokobanMapTest, testXSokoban0012) { testSingleMap("xsokoban0012"); }
TEST_F(SokobanMapTest, testXSokoban0013) { testSingleMap("xsokoban0013"); }
TEST_F(SokobanMapTest, testXSokoban0014) { testSingleMap("xsokoban0014"); }
TEST_F(SokobanMapTest, testXSokoban0015) { testSingleMap("xsokoban0015"); }
TEST_F(SokobanMapTest, testXSokoban0016) { testSingleMap("xsokoban0016"); }
TEST_F(SokobanMapTest, testXSokoban0017) { testSingleMap("xsokoban0017"); }
TEST_F(SokobanMapTest, testXSokoban0018) { testSingleMap("xsokoban0018"); }
TEST_F(SokobanMapTest, testXSokoban0019) { testSingleMap("xsokoban0019"); }
TEST_F(SokobanMapTest, testXSokoban0020) { testSingleMap("xsokoban0020"); }
TEST_F(SokobanMapTest, testXSokoban0021) { testSingleMap("xsokoban0021"); }
TEST_F(SokobanMapTest, testXSokoban0022) { testSingleMap("xsokoban0022"); }
TEST_F(SokobanMapTest, testXSokoban0023) { testSingleMap("xsokoban0023"); }
TEST_F(SokobanMapTest, testXSokoban0024) { testSingleMap("xsokoban0024"); }
TEST_F(SokobanMapTest, testXSokoban0025) { testSingleMap("xsokoban0025"); }
TEST_F(SokobanMapTest, testXSokoban0026) { testSingleMap("xsokoban0026"); }
TEST_F(SokobanMapTest, testXSokoban0027) { testSingleMap("xsokoban0027"); }
TEST_F(SokobanMapTest, testXSokoban0028) { testSingleMap("xsokoban0028"); }
TEST_F(SokobanMapTest, testXSokoban0029) { testSingleMap("xsokoban0029"); }
TEST_F(SokobanMapTest, testXSokoban0030) { testSingleMap("xsokoban0030"); }
TEST_F(SokobanMapTest, testXSokoban0031) { testSingleMap("xsokoban0031"); }
TEST_F(SokobanMapTest, testXSokoban0032) { testSingleMap("xsokoban0032"); }
TEST_F(SokobanMapTest, testXSokoban0033) { testSingleMap("xsokoban0033"); }
TEST_F(SokobanMapTest, testXSokoban0034) { testSingleMap("xsokoban0034"); }
TEST_F(SokobanMapTest, testXSokoban0035) { testSingleMap("xsokoban0035"); }
TEST_F(SokobanMapTest, testXSokoban0036) { testSingleMap("xsokoban0036"); }
TEST_F(SokobanMapTest, testXSokoban0037) { testSingleMap("xsokoban0037"); }
TEST_F(SokobanMapTest, testXSokoban0038) { testSingleMap("xsokoban0038"); }
TEST_F(SokobanMapTest, testXSokoban0039) { testSingleMap("xsokoban0039"); }
TEST_F(SokobanMapTest, testXSokoban0040) { testSingleMap("xsokoban0040"); }
TEST_F(SokobanMapTest, testXSokoban0041) { testSingleMap("xsokoban0041"); }
TEST_F(SokobanMapTest, testXSokoban0042) { testSingleMap("xsokoban0042"); }
TEST_F(SokobanMapTest, testXSokoban0043) { testSingleMap("xsokoban0043"); }
TEST_F(SokobanMapTest, testXSokoban0044) { testSingleMap("xsokoban0044"); }
TEST_F(SokobanMapTest, testXSokoban0045) { testSingleMap("xsokoban0045"); }
TEST_F(SokobanMapTest, testXSokoban0046) { testSingleMap("xsokoban0046"); }
TEST_F(SokobanMapTest, testXSokoban0047) { testSingleMap("xsokoban0047"); }
TEST_F(SokobanMapTest, testXSokoban0048) { testSingleMap("xsokoban0048"); }
TEST_F(SokobanMapTest, testXSokoban0049) { testSingleMap("xsokoban0049"); }
TEST_F(SokobanMapTest, testXSokoban0050) { testSingleMap("xsokoban0050"); }
TEST_F(SokobanMapTest, testXSokoban0051) { testSingleMap("xsokoban0051"); }
TEST_F(SokobanMapTest, testXSokoban0052) { testSingleMap("xsokoban0052"); }
// this map segfaults - but only if no other map was loaded before - if we add one or remove one tile, it works
TEST_F(SokobanMapTest, testXSokoban0053) { testSingleMap("xsokoban0053"); }
TEST_F(SokobanMapTest, testXSokoban0054) { testSingleMap("xsokoban0054"); }
// this map segfaults - but only if no other map was loaded before - if we add one or remove one tile, it works
TEST_F(SokobanMapTest, testXSokoban0055) { testSingleMap("xsokoban0055"); }
TEST_F(SokobanMapTest, testXSokoban0056) { testSingleMap("xsokoban0056"); }
TEST_F(SokobanMapTest, testXSokoban0057) { testSingleMap("xsokoban0057"); }
TEST_F(SokobanMapTest, testXSokoban0058) { testSingleMap("xsokoban0058"); }
TEST_F(SokobanMapTest, testXSokoban0059) { testSingleMap("xsokoban0059"); }
// this map segfaults - but only if no other map was loaded before - if we add one or remove one tile, it works
TEST_F(SokobanMapTest, testXSokoban0060) { testSingleMap("xsokoban0060"); }
TEST_F(SokobanMapTest, testXSokoban0061) { testSingleMap("xsokoban0061"); }
TEST_F(SokobanMapTest, testXSokoban0062) { testSingleMap("xsokoban0062"); }
TEST_F(SokobanMapTest, testXSokoban0063) { testSingleMap("xsokoban0063"); }
TEST_F(SokobanMapTest, testXSokoban0064) { testSingleMap("xsokoban0064"); }
TEST_F(SokobanMapTest, testXSokoban0065) { testSingleMap("xsokoban0065"); }
TEST_F(SokobanMapTest, testXSokoban0066) { testSingleMap("xsokoban0066"); }
TEST_F(SokobanMapTest, testXSokoban0067) { testSingleMap("xsokoban0067"); }
TEST_F(SokobanMapTest, testXSokoban0068) { testSingleMap("xsokoban0068"); }
TEST_F(SokobanMapTest, testXSokoban0069) { testSingleMap("xsokoban0069"); }
TEST_F(SokobanMapTest, testXSokoban0070) { testSingleMap("xsokoban0070"); }
TEST_F(SokobanMapTest, testXSokoban0071) { testSingleMap("xsokoban0071"); }
TEST_F(SokobanMapTest, testXSokoban0072) { testSingleMap("xsokoban0072"); }
TEST_F(SokobanMapTest, testXSokoban0073) { testSingleMap("xsokoban0073"); }
TEST_F(SokobanMapTest, testXSokoban0074) { testSingleMap("xsokoban0074"); }
TEST_F(SokobanMapTest, testXSokoban0075) { testSingleMap("xsokoban0075"); }
TEST_F(SokobanMapTest, testXSokoban0076) { testSingleMap("xsokoban0076"); }
TEST_F(SokobanMapTest, testXSokoban0077) { testSingleMap("xsokoban0077"); }
TEST_F(SokobanMapTest, testXSokoban0078) { testSingleMap("xsokoban0078"); }
TEST_F(SokobanMapTest, testXSokoban0079) { testSingleMap("xsokoban0079"); }
TEST_F(SokobanMapTest, testXSokoban0080) { testSingleMap("xsokoban0080"); }
TEST_F(SokobanMapTest, testXSokoban0081) { testSingleMap("xsokoban0081"); }
TEST_F(SokobanMapTest, testXSokoban0082) { testSingleMap("xsokoban0082"); }
TEST_F(SokobanMapTest, testXSokoban0083) { testSingleMap("xsokoban0083"); }
TEST_F(SokobanMapTest, testXSokoban0084) { testSingleMap("xsokoban0084"); }
TEST_F(SokobanMapTest, testXSokoban0085) { testSingleMap("xsokoban0085"); }
TEST_F(SokobanMapTest, testXSokoban0086) { testSingleMap("xsokoban0086"); }
TEST_F(SokobanMapTest, testXSokoban0087) { testSingleMap("xsokoban0087"); }
TEST_F(SokobanMapTest, testXSokoban0088) { testSingleMap("xsokoban0088"); }
TEST_F(SokobanMapTest, testXSokoban0089) { testSingleMap("xsokoban0089"); }
TEST_F(SokobanMapTest, testXSokoban0090) { testSingleMap("xsokoban0090"); }

TEST_F(SokobanMapTest, testMaps10)
{
	testMapRegion(1);
}

TEST_F(SokobanMapTest, testMaps20)
{
	testMapRegion(11);
}

TEST_F(SokobanMapTest, testMaps30)
{
	testMapRegion(21);
}

TEST_F(SokobanMapTest, testMaps40)
{
	testMapRegion(31);
}

TEST_F(SokobanMapTest, testMaps50)
{
	testMapRegion(41);
}

TEST_F(SokobanMapTest, testMaps60)
{
	testMapRegion(51);
}

TEST_F(SokobanMapTest, testMaps70)
{
	testMapRegion(61);
}

TEST_F(SokobanMapTest, testMaps80)
{
	testMapRegion(71);
}

TEST_F(SokobanMapTest, testMaps90)
{
	testMapRegion(81);
}

#include <fstream>
#include <iostream>

TEST_F(SokobanMapTest, DISABLED_convert)
{
	const int l= SDL_arraysize(level_data_);
	int map = 0;
	std::string m;
	for (int i = 0; i < l; ++i) {
		if (level_data_[i] == '\0') {
			++i;
			const std::string f = String::format("ksokoban%04i", map++);
			std::ofstream out(f);
			out << m;
			out.close();
			m = "";
			continue;
		}
		m.push_back(level_data_[i]);
	}
	const std::string f = String::format("ksokoban%04i", map++);
	std::ofstream out(f);
	out << m;
	out.close();
}
