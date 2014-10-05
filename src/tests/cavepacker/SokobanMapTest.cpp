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
TEST_F(SokobanMapTest, testksokoban0000) { testSingleMap("ksokoban0000"); }
TEST_F(SokobanMapTest, testksokoban0001) { testSingleMap("ksokoban0001"); }
TEST_F(SokobanMapTest, testksokoban0002) { testSingleMap("ksokoban0002"); }
TEST_F(SokobanMapTest, testksokoban0003) { testSingleMap("ksokoban0003"); }
TEST_F(SokobanMapTest, testksokoban0004) { testSingleMap("ksokoban0004"); }
TEST_F(SokobanMapTest, testksokoban0005) { testSingleMap("ksokoban0005"); }
TEST_F(SokobanMapTest, testksokoban0006) { testSingleMap("ksokoban0006"); }
TEST_F(SokobanMapTest, testksokoban0007) { testSingleMap("ksokoban0007"); }
TEST_F(SokobanMapTest, testksokoban0008) { testSingleMap("ksokoban0008"); }
TEST_F(SokobanMapTest, testksokoban0009) { testSingleMap("ksokoban0009"); }
TEST_F(SokobanMapTest, testksokoban0010) { testSingleMap("ksokoban0010"); }
TEST_F(SokobanMapTest, testksokoban0011) { testSingleMap("ksokoban0011"); }
TEST_F(SokobanMapTest, testksokoban0012) { testSingleMap("ksokoban0012"); }
TEST_F(SokobanMapTest, testksokoban0013) { testSingleMap("ksokoban0013"); }
TEST_F(SokobanMapTest, testksokoban0014) { testSingleMap("ksokoban0014"); }
TEST_F(SokobanMapTest, testksokoban0015) { testSingleMap("ksokoban0015"); }
TEST_F(SokobanMapTest, testksokoban0016) { testSingleMap("ksokoban0016"); }
TEST_F(SokobanMapTest, testksokoban0017) { testSingleMap("ksokoban0017"); }
TEST_F(SokobanMapTest, testksokoban0018) { testSingleMap("ksokoban0018"); }
TEST_F(SokobanMapTest, testksokoban0019) { testSingleMap("ksokoban0019"); }
TEST_F(SokobanMapTest, testksokoban0020) { testSingleMap("ksokoban0020"); }
TEST_F(SokobanMapTest, testksokoban0021) { testSingleMap("ksokoban0021"); }
TEST_F(SokobanMapTest, testksokoban0022) { testSingleMap("ksokoban0022"); }
TEST_F(SokobanMapTest, testksokoban0023) { testSingleMap("ksokoban0023"); }
TEST_F(SokobanMapTest, testksokoban0024) { testSingleMap("ksokoban0024"); }
TEST_F(SokobanMapTest, testksokoban0025) { testSingleMap("ksokoban0025"); }
TEST_F(SokobanMapTest, testksokoban0026) { testSingleMap("ksokoban0026"); }
TEST_F(SokobanMapTest, testksokoban0027) { testSingleMap("ksokoban0027"); }
TEST_F(SokobanMapTest, testksokoban0028) { testSingleMap("ksokoban0028"); }
TEST_F(SokobanMapTest, testksokoban0029) { testSingleMap("ksokoban0029"); }
TEST_F(SokobanMapTest, testksokoban0030) { testSingleMap("ksokoban0030"); }
TEST_F(SokobanMapTest, testksokoban0031) { testSingleMap("ksokoban0031"); }
TEST_F(SokobanMapTest, testksokoban0032) { testSingleMap("ksokoban0032"); }
TEST_F(SokobanMapTest, testksokoban0033) { testSingleMap("ksokoban0033"); }
TEST_F(SokobanMapTest, testksokoban0034) { testSingleMap("ksokoban0034"); }
TEST_F(SokobanMapTest, testksokoban0035) { testSingleMap("ksokoban0035"); }
TEST_F(SokobanMapTest, testksokoban0036) { testSingleMap("ksokoban0036"); }
TEST_F(SokobanMapTest, testksokoban0037) { testSingleMap("ksokoban0037"); }
TEST_F(SokobanMapTest, testksokoban0038) { testSingleMap("ksokoban0038"); }
TEST_F(SokobanMapTest, testksokoban0039) { testSingleMap("ksokoban0039"); }
TEST_F(SokobanMapTest, testksokoban0040) { testSingleMap("ksokoban0040"); }
TEST_F(SokobanMapTest, testksokoban0041) { testSingleMap("ksokoban0041"); }
TEST_F(SokobanMapTest, testksokoban0042) { testSingleMap("ksokoban0042"); }
TEST_F(SokobanMapTest, testksokoban0043) { testSingleMap("ksokoban0043"); }
TEST_F(SokobanMapTest, testksokoban0044) { testSingleMap("ksokoban0044"); }
TEST_F(SokobanMapTest, testksokoban0045) { testSingleMap("ksokoban0045"); }
TEST_F(SokobanMapTest, testksokoban0046) { testSingleMap("ksokoban0046"); }
TEST_F(SokobanMapTest, testksokoban0047) { testSingleMap("ksokoban0047"); }
TEST_F(SokobanMapTest, testksokoban0048) { testSingleMap("ksokoban0048"); }
TEST_F(SokobanMapTest, testksokoban0049) { testSingleMap("ksokoban0049"); }
TEST_F(SokobanMapTest, testksokoban0050) { testSingleMap("ksokoban0050"); }
TEST_F(SokobanMapTest, testksokoban0051) { testSingleMap("ksokoban0051"); }
TEST_F(SokobanMapTest, testksokoban0052) { testSingleMap("ksokoban0052"); }
TEST_F(SokobanMapTest, testksokoban0053) { testSingleMap("ksokoban0053"); }
TEST_F(SokobanMapTest, testksokoban0054) { testSingleMap("ksokoban0054"); }
TEST_F(SokobanMapTest, testksokoban0055) { testSingleMap("ksokoban0055"); }
TEST_F(SokobanMapTest, testksokoban0056) { testSingleMap("ksokoban0056"); }
TEST_F(SokobanMapTest, testksokoban0057) { testSingleMap("ksokoban0057"); }
TEST_F(SokobanMapTest, testksokoban0058) { testSingleMap("ksokoban0058"); }
TEST_F(SokobanMapTest, testksokoban0059) { testSingleMap("ksokoban0059"); }
TEST_F(SokobanMapTest, testksokoban0060) { testSingleMap("ksokoban0060"); }
TEST_F(SokobanMapTest, testksokoban0061) { testSingleMap("ksokoban0061"); }
TEST_F(SokobanMapTest, testksokoban0062) { testSingleMap("ksokoban0062"); }
TEST_F(SokobanMapTest, testksokoban0063) { testSingleMap("ksokoban0063"); }
TEST_F(SokobanMapTest, testksokoban0064) { testSingleMap("ksokoban0064"); }
TEST_F(SokobanMapTest, testksokoban0065) { testSingleMap("ksokoban0065"); }
TEST_F(SokobanMapTest, testksokoban0066) { testSingleMap("ksokoban0066"); }
TEST_F(SokobanMapTest, testksokoban0067) { testSingleMap("ksokoban0067"); }
TEST_F(SokobanMapTest, testksokoban0068) { testSingleMap("ksokoban0068"); }
TEST_F(SokobanMapTest, testksokoban0069) { testSingleMap("ksokoban0069"); }
TEST_F(SokobanMapTest, testksokoban0070) { testSingleMap("ksokoban0070"); }
TEST_F(SokobanMapTest, testksokoban0071) { testSingleMap("ksokoban0071"); }
TEST_F(SokobanMapTest, testksokoban0072) { testSingleMap("ksokoban0072"); }
TEST_F(SokobanMapTest, testksokoban0073) { testSingleMap("ksokoban0073"); }
TEST_F(SokobanMapTest, testksokoban0074) { testSingleMap("ksokoban0074"); }
TEST_F(SokobanMapTest, testksokoban0075) { testSingleMap("ksokoban0075"); }
TEST_F(SokobanMapTest, testksokoban0076) { testSingleMap("ksokoban0076"); }
TEST_F(SokobanMapTest, testksokoban0077) { testSingleMap("ksokoban0077"); }
TEST_F(SokobanMapTest, testksokoban0078) { testSingleMap("ksokoban0078"); }
TEST_F(SokobanMapTest, testksokoban0079) { testSingleMap("ksokoban0079"); }
TEST_F(SokobanMapTest, testksokoban0080) { testSingleMap("ksokoban0080"); }
TEST_F(SokobanMapTest, testksokoban0081) { testSingleMap("ksokoban0081"); }
TEST_F(SokobanMapTest, testksokoban0082) { testSingleMap("ksokoban0082"); }
TEST_F(SokobanMapTest, testksokoban0083) { testSingleMap("ksokoban0083"); }
TEST_F(SokobanMapTest, testksokoban0084) { testSingleMap("ksokoban0084"); }
TEST_F(SokobanMapTest, testksokoban0085) { testSingleMap("ksokoban0085"); }
TEST_F(SokobanMapTest, testksokoban0086) { testSingleMap("ksokoban0086"); }
TEST_F(SokobanMapTest, testksokoban0087) { testSingleMap("ksokoban0087"); }
TEST_F(SokobanMapTest, testksokoban0088) { testSingleMap("ksokoban0088"); }
TEST_F(SokobanMapTest, testksokoban0089) { testSingleMap("ksokoban0089"); }
TEST_F(SokobanMapTest, testksokoban0090) { testSingleMap("ksokoban0090"); }
TEST_F(SokobanMapTest, testksokoban0091) { testSingleMap("ksokoban0091"); }
TEST_F(SokobanMapTest, testksokoban0092) { testSingleMap("ksokoban0092"); }
TEST_F(SokobanMapTest, testksokoban0093) { testSingleMap("ksokoban0093"); }
TEST_F(SokobanMapTest, testksokoban0094) { testSingleMap("ksokoban0094"); }
TEST_F(SokobanMapTest, testksokoban0095) { testSingleMap("ksokoban0095"); }
TEST_F(SokobanMapTest, testksokoban0096) { testSingleMap("ksokoban0096"); }
TEST_F(SokobanMapTest, testksokoban0097) { testSingleMap("ksokoban0097"); }
TEST_F(SokobanMapTest, testksokoban0098) { testSingleMap("ksokoban0098"); }
TEST_F(SokobanMapTest, testksokoban0099) { testSingleMap("ksokoban0099"); }
TEST_F(SokobanMapTest, testksokoban0100) { testSingleMap("ksokoban0100"); }
TEST_F(SokobanMapTest, testksokoban0101) { testSingleMap("ksokoban0101"); }
TEST_F(SokobanMapTest, testksokoban0102) { testSingleMap("ksokoban0102"); }
TEST_F(SokobanMapTest, testksokoban0103) { testSingleMap("ksokoban0103"); }
TEST_F(SokobanMapTest, testksokoban0104) { testSingleMap("ksokoban0104"); }
TEST_F(SokobanMapTest, testksokoban0105) { testSingleMap("ksokoban0105"); }
TEST_F(SokobanMapTest, testksokoban0106) { testSingleMap("ksokoban0106"); }
TEST_F(SokobanMapTest, testksokoban0107) { testSingleMap("ksokoban0107"); }
TEST_F(SokobanMapTest, testksokoban0108) { testSingleMap("ksokoban0108"); }
TEST_F(SokobanMapTest, testksokoban0109) { testSingleMap("ksokoban0109"); }
TEST_F(SokobanMapTest, testksokoban0110) { testSingleMap("ksokoban0110"); }
TEST_F(SokobanMapTest, testksokoban0111) { testSingleMap("ksokoban0111"); }
TEST_F(SokobanMapTest, testksokoban0112) { testSingleMap("ksokoban0112"); }
TEST_F(SokobanMapTest, testksokoban0113) { testSingleMap("ksokoban0113"); }
TEST_F(SokobanMapTest, testksokoban0114) { testSingleMap("ksokoban0114"); }
TEST_F(SokobanMapTest, testksokoban0115) { testSingleMap("ksokoban0115"); }
TEST_F(SokobanMapTest, testksokoban0116) { testSingleMap("ksokoban0116"); }
TEST_F(SokobanMapTest, testksokoban0117) { testSingleMap("ksokoban0117"); }
TEST_F(SokobanMapTest, testksokoban0118) { testSingleMap("ksokoban0118"); }
TEST_F(SokobanMapTest, testksokoban0119) { testSingleMap("ksokoban0119"); }
TEST_F(SokobanMapTest, testksokoban0120) { testSingleMap("ksokoban0120"); }
TEST_F(SokobanMapTest, testksokoban0121) { testSingleMap("ksokoban0121"); }
TEST_F(SokobanMapTest, testksokoban0122) { testSingleMap("ksokoban0122"); }
TEST_F(SokobanMapTest, testksokoban0123) { testSingleMap("ksokoban0123"); }
TEST_F(SokobanMapTest, testksokoban0124) { testSingleMap("ksokoban0124"); }
TEST_F(SokobanMapTest, testksokoban0125) { testSingleMap("ksokoban0125"); }
TEST_F(SokobanMapTest, testksokoban0126) { testSingleMap("ksokoban0126"); }
TEST_F(SokobanMapTest, testksokoban0127) { testSingleMap("ksokoban0127"); }
TEST_F(SokobanMapTest, testksokoban0128) { testSingleMap("ksokoban0128"); }
TEST_F(SokobanMapTest, testksokoban0129) { testSingleMap("ksokoban0129"); }
TEST_F(SokobanMapTest, testksokoban0130) { testSingleMap("ksokoban0130"); }
TEST_F(SokobanMapTest, testksokoban0131) { testSingleMap("ksokoban0131"); }
TEST_F(SokobanMapTest, testksokoban0132) { testSingleMap("ksokoban0132"); }
TEST_F(SokobanMapTest, testksokoban0133) { testSingleMap("ksokoban0133"); }
TEST_F(SokobanMapTest, testksokoban0134) { testSingleMap("ksokoban0134"); }
TEST_F(SokobanMapTest, testksokoban0135) { testSingleMap("ksokoban0135"); }
TEST_F(SokobanMapTest, testksokoban0136) { testSingleMap("ksokoban0136"); }
TEST_F(SokobanMapTest, testksokoban0137) { testSingleMap("ksokoban0137"); }
TEST_F(SokobanMapTest, testksokoban0138) { testSingleMap("ksokoban0138"); }
TEST_F(SokobanMapTest, testksokoban0139) { testSingleMap("ksokoban0139"); }
TEST_F(SokobanMapTest, testksokoban0140) { testSingleMap("ksokoban0140"); }
TEST_F(SokobanMapTest, testksokoban0141) { testSingleMap("ksokoban0141"); }
TEST_F(SokobanMapTest, testksokoban0142) { testSingleMap("ksokoban0142"); }
TEST_F(SokobanMapTest, testksokoban0143) { testSingleMap("ksokoban0143"); }
TEST_F(SokobanMapTest, testksokoban0144) { testSingleMap("ksokoban0144"); }
TEST_F(SokobanMapTest, testksokoban0145) { testSingleMap("ksokoban0145"); }
TEST_F(SokobanMapTest, testksokoban0146) { testSingleMap("ksokoban0146"); }
TEST_F(SokobanMapTest, testksokoban0147) { testSingleMap("ksokoban0147"); }
TEST_F(SokobanMapTest, testksokoban0148) { testSingleMap("ksokoban0148"); }
TEST_F(SokobanMapTest, testksokoban0149) { testSingleMap("ksokoban0149"); }
TEST_F(SokobanMapTest, testksokoban0150) { testSingleMap("ksokoban0150"); }
TEST_F(SokobanMapTest, testksokoban0151) { testSingleMap("ksokoban0151"); }
TEST_F(SokobanMapTest, testksokoban0152) { testSingleMap("ksokoban0152"); }
TEST_F(SokobanMapTest, testksokoban0153) { testSingleMap("ksokoban0153"); }
TEST_F(SokobanMapTest, testksokoban0154) { testSingleMap("ksokoban0154"); }
TEST_F(SokobanMapTest, testksokoban0155) { testSingleMap("ksokoban0155"); }
TEST_F(SokobanMapTest, testksokoban0156) { testSingleMap("ksokoban0156"); }
TEST_F(SokobanMapTest, testksokoban0157) { testSingleMap("ksokoban0157"); }
TEST_F(SokobanMapTest, testksokoban0158) { testSingleMap("ksokoban0158"); }
TEST_F(SokobanMapTest, testksokoban0159) { testSingleMap("ksokoban0159"); }
TEST_F(SokobanMapTest, testksokoban0160) { testSingleMap("ksokoban0160"); }
TEST_F(SokobanMapTest, testksokoban0161) { testSingleMap("ksokoban0161"); }
TEST_F(SokobanMapTest, testksokoban0162) { testSingleMap("ksokoban0162"); }
TEST_F(SokobanMapTest, testksokoban0163) { testSingleMap("ksokoban0163"); }
TEST_F(SokobanMapTest, testksokoban0164) { testSingleMap("ksokoban0164"); }
TEST_F(SokobanMapTest, testksokoban0165) { testSingleMap("ksokoban0165"); }
TEST_F(SokobanMapTest, testksokoban0166) { testSingleMap("ksokoban0166"); }
TEST_F(SokobanMapTest, testksokoban0167) { testSingleMap("ksokoban0167"); }
TEST_F(SokobanMapTest, testksokoban0168) { testSingleMap("ksokoban0168"); }
TEST_F(SokobanMapTest, testksokoban0169) { testSingleMap("ksokoban0169"); }
TEST_F(SokobanMapTest, testksokoban0170) { testSingleMap("ksokoban0170"); }
TEST_F(SokobanMapTest, testksokoban0171) { testSingleMap("ksokoban0171"); }
TEST_F(SokobanMapTest, testksokoban0172) { testSingleMap("ksokoban0172"); }
TEST_F(SokobanMapTest, testksokoban0173) { testSingleMap("ksokoban0173"); }
TEST_F(SokobanMapTest, testksokoban0174) { testSingleMap("ksokoban0174"); }
TEST_F(SokobanMapTest, testksokoban0175) { testSingleMap("ksokoban0175"); }
TEST_F(SokobanMapTest, testksokoban0176) { testSingleMap("ksokoban0176"); }
TEST_F(SokobanMapTest, testksokoban0177) { testSingleMap("ksokoban0177"); }
TEST_F(SokobanMapTest, testksokoban0178) { testSingleMap("ksokoban0178"); }
TEST_F(SokobanMapTest, testksokoban0179) { testSingleMap("ksokoban0179"); }
TEST_F(SokobanMapTest, testksokoban0180) { testSingleMap("ksokoban0180"); }
TEST_F(SokobanMapTest, testksokoban0181) { testSingleMap("ksokoban0181"); }
TEST_F(SokobanMapTest, testksokoban0182) { testSingleMap("ksokoban0182"); }
TEST_F(SokobanMapTest, testksokoban0183) { testSingleMap("ksokoban0183"); }
TEST_F(SokobanMapTest, testksokoban0184) { testSingleMap("ksokoban0184"); }
TEST_F(SokobanMapTest, testksokoban0185) { testSingleMap("ksokoban0185"); }
TEST_F(SokobanMapTest, testksokoban0186) { testSingleMap("ksokoban0186"); }
TEST_F(SokobanMapTest, testksokoban0187) { testSingleMap("ksokoban0187"); }
TEST_F(SokobanMapTest, testksokoban0188) { testSingleMap("ksokoban0188"); }
TEST_F(SokobanMapTest, testksokoban0189) { testSingleMap("ksokoban0189"); }
TEST_F(SokobanMapTest, testksokoban0190) { testSingleMap("ksokoban0190"); }
TEST_F(SokobanMapTest, testksokoban0191) { testSingleMap("ksokoban0191"); }
TEST_F(SokobanMapTest, testksokoban0192) { testSingleMap("ksokoban0192"); }
TEST_F(SokobanMapTest, testksokoban0193) { testSingleMap("ksokoban0193"); }
TEST_F(SokobanMapTest, testksokoban0194) { testSingleMap("ksokoban0194"); }
TEST_F(SokobanMapTest, testksokoban0195) { testSingleMap("ksokoban0195"); }
TEST_F(SokobanMapTest, testksokoban0196) { testSingleMap("ksokoban0196"); }
TEST_F(SokobanMapTest, testksokoban0197) { testSingleMap("ksokoban0197"); }
TEST_F(SokobanMapTest, testksokoban0198) { testSingleMap("ksokoban0198"); }
TEST_F(SokobanMapTest, testksokoban0199) { testSingleMap("ksokoban0199"); }
TEST_F(SokobanMapTest, testksokoban0200) { testSingleMap("ksokoban0200"); }
TEST_F(SokobanMapTest, testksokoban0201) { testSingleMap("ksokoban0201"); }
TEST_F(SokobanMapTest, testksokoban0202) { testSingleMap("ksokoban0202"); }
TEST_F(SokobanMapTest, testksokoban0203) { testSingleMap("ksokoban0203"); }
TEST_F(SokobanMapTest, testksokoban0204) { testSingleMap("ksokoban0204"); }
TEST_F(SokobanMapTest, testksokoban0205) { testSingleMap("ksokoban0205"); }
TEST_F(SokobanMapTest, testksokoban0206) { testSingleMap("ksokoban0206"); }
TEST_F(SokobanMapTest, testksokoban0207) { testSingleMap("ksokoban0207"); }
TEST_F(SokobanMapTest, testksokoban0208) { testSingleMap("ksokoban0208"); }
TEST_F(SokobanMapTest, testksokoban0209) { testSingleMap("ksokoban0209"); }
TEST_F(SokobanMapTest, testksokoban0210) { testSingleMap("ksokoban0210"); }
TEST_F(SokobanMapTest, testksokoban0211) { testSingleMap("ksokoban0211"); }
TEST_F(SokobanMapTest, testksokoban0212) { testSingleMap("ksokoban0212"); }
TEST_F(SokobanMapTest, testksokoban0213) { testSingleMap("ksokoban0213"); }
TEST_F(SokobanMapTest, testksokoban0214) { testSingleMap("ksokoban0214"); }
TEST_F(SokobanMapTest, testksokoban0215) { testSingleMap("ksokoban0215"); }
TEST_F(SokobanMapTest, testksokoban0216) { testSingleMap("ksokoban0216"); }
TEST_F(SokobanMapTest, testksokoban0217) { testSingleMap("ksokoban0217"); }
TEST_F(SokobanMapTest, testksokoban0218) { testSingleMap("ksokoban0218"); }
TEST_F(SokobanMapTest, testksokoban0219) { testSingleMap("ksokoban0219"); }
TEST_F(SokobanMapTest, testksokoban0220) { testSingleMap("ksokoban0220"); }
TEST_F(SokobanMapTest, testksokoban0221) { testSingleMap("ksokoban0221"); }
TEST_F(SokobanMapTest, testksokoban0222) { testSingleMap("ksokoban0222"); }
TEST_F(SokobanMapTest, testksokoban0223) { testSingleMap("ksokoban0223"); }
TEST_F(SokobanMapTest, testksokoban0224) { testSingleMap("ksokoban0224"); }
TEST_F(SokobanMapTest, testksokoban0225) { testSingleMap("ksokoban0225"); }
TEST_F(SokobanMapTest, testksokoban0226) { testSingleMap("ksokoban0226"); }
TEST_F(SokobanMapTest, testksokoban0227) { testSingleMap("ksokoban0227"); }
TEST_F(SokobanMapTest, testksokoban0228) { testSingleMap("ksokoban0228"); }
TEST_F(SokobanMapTest, testksokoban0229) { testSingleMap("ksokoban0229"); }
TEST_F(SokobanMapTest, testksokoban0230) { testSingleMap("ksokoban0230"); }
TEST_F(SokobanMapTest, testksokoban0231) { testSingleMap("ksokoban0231"); }
TEST_F(SokobanMapTest, testksokoban0232) { testSingleMap("ksokoban0232"); }
TEST_F(SokobanMapTest, testksokoban0233) { testSingleMap("ksokoban0233"); }
TEST_F(SokobanMapTest, testksokoban0234) { testSingleMap("ksokoban0234"); }
TEST_F(SokobanMapTest, testksokoban0235) { testSingleMap("ksokoban0235"); }
TEST_F(SokobanMapTest, testksokoban0236) { testSingleMap("ksokoban0236"); }
TEST_F(SokobanMapTest, testksokoban0237) { testSingleMap("ksokoban0237"); }
TEST_F(SokobanMapTest, testksokoban0238) { testSingleMap("ksokoban0238"); }
TEST_F(SokobanMapTest, testksokoban0239) { testSingleMap("ksokoban0239"); }
TEST_F(SokobanMapTest, testksokoban0240) { testSingleMap("ksokoban0240"); }
TEST_F(SokobanMapTest, testksokoban0241) { testSingleMap("ksokoban0241"); }
TEST_F(SokobanMapTest, testksokoban0242) { testSingleMap("ksokoban0242"); }
TEST_F(SokobanMapTest, testksokoban0243) { testSingleMap("ksokoban0243"); }
TEST_F(SokobanMapTest, testksokoban0244) { testSingleMap("ksokoban0244"); }
TEST_F(SokobanMapTest, testksokoban0245) { testSingleMap("ksokoban0245"); }
TEST_F(SokobanMapTest, testksokoban0246) { testSingleMap("ksokoban0246"); }
TEST_F(SokobanMapTest, testksokoban0247) { testSingleMap("ksokoban0247"); }
TEST_F(SokobanMapTest, testksokoban0248) { testSingleMap("ksokoban0248"); }
TEST_F(SokobanMapTest, testksokoban0249) { testSingleMap("ksokoban0249"); }
TEST_F(SokobanMapTest, testksokoban0250) { testSingleMap("ksokoban0250"); }
TEST_F(SokobanMapTest, testksokoban0251) { testSingleMap("ksokoban0251"); }
TEST_F(SokobanMapTest, testksokoban0252) { testSingleMap("ksokoban0252"); }
TEST_F(SokobanMapTest, testksokoban0253) { testSingleMap("ksokoban0253"); }
TEST_F(SokobanMapTest, testksokoban0254) { testSingleMap("ksokoban0254"); }
TEST_F(SokobanMapTest, testksokoban0255) { testSingleMap("ksokoban0255"); }
TEST_F(SokobanMapTest, testksokoban0256) { testSingleMap("ksokoban0256"); }
TEST_F(SokobanMapTest, testksokoban0257) { testSingleMap("ksokoban0257"); }
TEST_F(SokobanMapTest, testksokoban0258) { testSingleMap("ksokoban0258"); }
TEST_F(SokobanMapTest, testksokoban0259) { testSingleMap("ksokoban0259"); }
TEST_F(SokobanMapTest, testksokoban0260) { testSingleMap("ksokoban0260"); }
TEST_F(SokobanMapTest, testksokoban0261) { testSingleMap("ksokoban0261"); }
TEST_F(SokobanMapTest, testksokoban0262) { testSingleMap("ksokoban0262"); }
TEST_F(SokobanMapTest, testksokoban0263) { testSingleMap("ksokoban0263"); }
TEST_F(SokobanMapTest, testksokoban0264) { testSingleMap("ksokoban0264"); }
TEST_F(SokobanMapTest, testksokoban0265) { testSingleMap("ksokoban0265"); }
TEST_F(SokobanMapTest, testksokoban0266) { testSingleMap("ksokoban0266"); }
TEST_F(SokobanMapTest, testksokoban0267) { testSingleMap("ksokoban0267"); }
TEST_F(SokobanMapTest, testksokoban0268) { testSingleMap("ksokoban0268"); }
TEST_F(SokobanMapTest, testksokoban0269) { testSingleMap("ksokoban0269"); }
TEST_F(SokobanMapTest, testksokoban0270) { testSingleMap("ksokoban0270"); }
TEST_F(SokobanMapTest, testksokoban0271) { testSingleMap("ksokoban0271"); }
TEST_F(SokobanMapTest, testksokoban0272) { testSingleMap("ksokoban0272"); }
TEST_F(SokobanMapTest, testksokoban0273) { testSingleMap("ksokoban0273"); }
TEST_F(SokobanMapTest, testksokoban0274) { testSingleMap("ksokoban0274"); }
TEST_F(SokobanMapTest, testksokoban0275) { testSingleMap("ksokoban0275"); }
TEST_F(SokobanMapTest, testksokoban0276) { testSingleMap("ksokoban0276"); }
TEST_F(SokobanMapTest, testksokoban0277) { testSingleMap("ksokoban0277"); }
TEST_F(SokobanMapTest, testksokoban0278) { testSingleMap("ksokoban0278"); }
TEST_F(SokobanMapTest, testksokoban0279) { testSingleMap("ksokoban0279"); }
TEST_F(SokobanMapTest, testksokoban0280) { testSingleMap("ksokoban0280"); }
TEST_F(SokobanMapTest, testksokoban0281) { testSingleMap("ksokoban0281"); }
TEST_F(SokobanMapTest, testksokoban0282) { testSingleMap("ksokoban0282"); }
TEST_F(SokobanMapTest, testksokoban0283) { testSingleMap("ksokoban0283"); }
TEST_F(SokobanMapTest, testksokoban0284) { testSingleMap("ksokoban0284"); }
TEST_F(SokobanMapTest, testksokoban0285) { testSingleMap("ksokoban0285"); }
TEST_F(SokobanMapTest, testksokoban0286) { testSingleMap("ksokoban0286"); }
TEST_F(SokobanMapTest, testksokoban0287) { testSingleMap("ksokoban0287"); }
TEST_F(SokobanMapTest, testksokoban0288) { testSingleMap("ksokoban0288"); }
TEST_F(SokobanMapTest, testksokoban0289) { testSingleMap("ksokoban0289"); }
TEST_F(SokobanMapTest, testksokoban0290) { testSingleMap("ksokoban0290"); }
TEST_F(SokobanMapTest, testksokoban0291) { testSingleMap("ksokoban0291"); }
TEST_F(SokobanMapTest, testksokoban0292) { testSingleMap("ksokoban0292"); }
TEST_F(SokobanMapTest, testksokoban0293) { testSingleMap("ksokoban0293"); }
TEST_F(SokobanMapTest, testksokoban0294) { testSingleMap("ksokoban0294"); }
TEST_F(SokobanMapTest, testksokoban0295) { testSingleMap("ksokoban0295"); }
TEST_F(SokobanMapTest, testksokoban0296) { testSingleMap("ksokoban0296"); }
TEST_F(SokobanMapTest, testksokoban0297) { testSingleMap("ksokoban0297"); }
TEST_F(SokobanMapTest, testksokoban0298) { testSingleMap("ksokoban0298"); }
TEST_F(SokobanMapTest, testksokoban0299) { testSingleMap("ksokoban0299"); }
TEST_F(SokobanMapTest, testksokoban0300) { testSingleMap("ksokoban0300"); }
TEST_F(SokobanMapTest, testksokoban0301) { testSingleMap("ksokoban0301"); }
TEST_F(SokobanMapTest, testksokoban0302) { testSingleMap("ksokoban0302"); }
TEST_F(SokobanMapTest, testksokoban0303) { testSingleMap("ksokoban0303"); }
TEST_F(SokobanMapTest, testksokoban0304) { testSingleMap("ksokoban0304"); }
TEST_F(SokobanMapTest, testksokoban0305) { testSingleMap("ksokoban0305"); }
TEST_F(SokobanMapTest, testksokoban0306) { testSingleMap("ksokoban0306"); }
TEST_F(SokobanMapTest, testksokoban0307) { testSingleMap("ksokoban0307"); }
TEST_F(SokobanMapTest, testksokoban0308) { testSingleMap("ksokoban0308"); }
TEST_F(SokobanMapTest, testksokoban0309) { testSingleMap("ksokoban0309"); }
TEST_F(SokobanMapTest, testksokoban0310) { testSingleMap("ksokoban0310"); }
TEST_F(SokobanMapTest, testksokoban0311) { testSingleMap("ksokoban0311"); }
TEST_F(SokobanMapTest, testksokoban0312) { testSingleMap("ksokoban0312"); }
TEST_F(SokobanMapTest, testksokoban0313) { testSingleMap("ksokoban0313"); }
TEST_F(SokobanMapTest, testksokoban0314) { testSingleMap("ksokoban0314"); }
TEST_F(SokobanMapTest, testksokoban0315) { testSingleMap("ksokoban0315"); }
TEST_F(SokobanMapTest, testksokoban0316) { testSingleMap("ksokoban0316"); }
TEST_F(SokobanMapTest, testksokoban0317) { testSingleMap("ksokoban0317"); }
TEST_F(SokobanMapTest, testksokoban0318) { testSingleMap("ksokoban0318"); }
TEST_F(SokobanMapTest, testksokoban0319) { testSingleMap("ksokoban0319"); }
TEST_F(SokobanMapTest, testksokoban0320) { testSingleMap("ksokoban0320"); }
TEST_F(SokobanMapTest, testksokoban0321) { testSingleMap("ksokoban0321"); }
TEST_F(SokobanMapTest, testksokoban0322) { testSingleMap("ksokoban0322"); }
TEST_F(SokobanMapTest, testksokoban0323) { testSingleMap("ksokoban0323"); }
TEST_F(SokobanMapTest, testksokoban0324) { testSingleMap("ksokoban0324"); }
TEST_F(SokobanMapTest, testksokoban0325) { testSingleMap("ksokoban0325"); }
TEST_F(SokobanMapTest, testksokoban0326) { testSingleMap("ksokoban0326"); }
TEST_F(SokobanMapTest, testksokoban0327) { testSingleMap("ksokoban0327"); }
TEST_F(SokobanMapTest, testksokoban0328) { testSingleMap("ksokoban0328"); }
TEST_F(SokobanMapTest, testksokoban0329) { testSingleMap("ksokoban0329"); }
TEST_F(SokobanMapTest, testksokoban0330) { testSingleMap("ksokoban0330"); }
TEST_F(SokobanMapTest, testksokoban0331) { testSingleMap("ksokoban0331"); }
TEST_F(SokobanMapTest, testksokoban0332) { testSingleMap("ksokoban0332"); }
TEST_F(SokobanMapTest, testksokoban0333) { testSingleMap("ksokoban0333"); }
TEST_F(SokobanMapTest, testksokoban0334) { testSingleMap("ksokoban0334"); }
TEST_F(SokobanMapTest, testksokoban0335) { testSingleMap("ksokoban0335"); }
TEST_F(SokobanMapTest, testksokoban0336) { testSingleMap("ksokoban0336"); }
TEST_F(SokobanMapTest, testksokoban0337) { testSingleMap("ksokoban0337"); }
TEST_F(SokobanMapTest, testksokoban0338) { testSingleMap("ksokoban0338"); }
TEST_F(SokobanMapTest, testksokoban0339) { testSingleMap("ksokoban0339"); }
TEST_F(SokobanMapTest, testksokoban0340) { testSingleMap("ksokoban0340"); }
TEST_F(SokobanMapTest, testksokoban0341) { testSingleMap("ksokoban0341"); }
TEST_F(SokobanMapTest, testksokoban0342) { testSingleMap("ksokoban0342"); }
TEST_F(SokobanMapTest, testksokoban0343) { testSingleMap("ksokoban0343"); }
TEST_F(SokobanMapTest, testksokoban0344) { testSingleMap("ksokoban0344"); }
TEST_F(SokobanMapTest, testksokoban0345) { testSingleMap("ksokoban0345"); }
TEST_F(SokobanMapTest, testksokoban0346) { testSingleMap("ksokoban0346"); }
TEST_F(SokobanMapTest, testksokoban0347) { testSingleMap("ksokoban0347"); }
TEST_F(SokobanMapTest, testksokoban0348) { testSingleMap("ksokoban0348"); }
TEST_F(SokobanMapTest, testksokoban0349) { testSingleMap("ksokoban0349"); }
TEST_F(SokobanMapTest, testksokoban0350) { testSingleMap("ksokoban0350"); }
TEST_F(SokobanMapTest, testksokoban0351) { testSingleMap("ksokoban0351"); }
TEST_F(SokobanMapTest, testksokoban0352) { testSingleMap("ksokoban0352"); }
TEST_F(SokobanMapTest, testksokoban0353) { testSingleMap("ksokoban0353"); }
TEST_F(SokobanMapTest, testksokoban0354) { testSingleMap("ksokoban0354"); }
TEST_F(SokobanMapTest, testgri01) { testSingleMap("gri01"); }
TEST_F(SokobanMapTest, testgri02) { testSingleMap("gri02"); }
TEST_F(SokobanMapTest, testgri03) { testSingleMap("gri03"); }
TEST_F(SokobanMapTest, testgri04) { testSingleMap("gri04"); }
TEST_F(SokobanMapTest, testgri05) { testSingleMap("gri05"); }
TEST_F(SokobanMapTest, testgri06) { testSingleMap("gri06"); }
TEST_F(SokobanMapTest, testgri07) { testSingleMap("gri07"); }
TEST_F(SokobanMapTest, testgri08) { testSingleMap("gri08"); }
TEST_F(SokobanMapTest, testgri09) { testSingleMap("gri09"); }
TEST_F(SokobanMapTest, testgri100) { testSingleMap("gri100"); }
TEST_F(SokobanMapTest, testgri101) { testSingleMap("gri101"); }
TEST_F(SokobanMapTest, testgri102) { testSingleMap("gri102"); }
TEST_F(SokobanMapTest, testgri103) { testSingleMap("gri103"); }
TEST_F(SokobanMapTest, testgri104) { testSingleMap("gri104"); }
TEST_F(SokobanMapTest, testgri105) { testSingleMap("gri105"); }
TEST_F(SokobanMapTest, testgri106) { testSingleMap("gri106"); }
TEST_F(SokobanMapTest, testgri107) { testSingleMap("gri107"); }
TEST_F(SokobanMapTest, testgri108) { testSingleMap("gri108"); }
TEST_F(SokobanMapTest, testgri109) { testSingleMap("gri109"); }
TEST_F(SokobanMapTest, testgri10) { testSingleMap("gri10"); }
TEST_F(SokobanMapTest, testgri110) { testSingleMap("gri110"); }
TEST_F(SokobanMapTest, testgri111) { testSingleMap("gri111"); }
TEST_F(SokobanMapTest, testgri112) { testSingleMap("gri112"); }
TEST_F(SokobanMapTest, testgri113) { testSingleMap("gri113"); }
TEST_F(SokobanMapTest, testgri114) { testSingleMap("gri114"); }
TEST_F(SokobanMapTest, testgri115) { testSingleMap("gri115"); }
TEST_F(SokobanMapTest, testgri116) { testSingleMap("gri116"); }
TEST_F(SokobanMapTest, testgri117) { testSingleMap("gri117"); }
TEST_F(SokobanMapTest, testgri118) { testSingleMap("gri118"); }
TEST_F(SokobanMapTest, testgri119) { testSingleMap("gri119"); }
TEST_F(SokobanMapTest, testgri11) { testSingleMap("gri11"); }
TEST_F(SokobanMapTest, testgri120) { testSingleMap("gri120"); }
TEST_F(SokobanMapTest, testgri121) { testSingleMap("gri121"); }
TEST_F(SokobanMapTest, testgri122) { testSingleMap("gri122"); }
TEST_F(SokobanMapTest, testgri123) { testSingleMap("gri123"); }
TEST_F(SokobanMapTest, testgri124) { testSingleMap("gri124"); }
TEST_F(SokobanMapTest, testgri125) { testSingleMap("gri125"); }
TEST_F(SokobanMapTest, testgri126) { testSingleMap("gri126"); }
TEST_F(SokobanMapTest, testgri127) { testSingleMap("gri127"); }
TEST_F(SokobanMapTest, testgri128) { testSingleMap("gri128"); }
TEST_F(SokobanMapTest, testgri129) { testSingleMap("gri129"); }
TEST_F(SokobanMapTest, testgri12) { testSingleMap("gri12"); }
TEST_F(SokobanMapTest, testgri130) { testSingleMap("gri130"); }
TEST_F(SokobanMapTest, testgri131) { testSingleMap("gri131"); }
TEST_F(SokobanMapTest, testgri132) { testSingleMap("gri132"); }
TEST_F(SokobanMapTest, testgri133) { testSingleMap("gri133"); }
TEST_F(SokobanMapTest, testgri134) { testSingleMap("gri134"); }
TEST_F(SokobanMapTest, testgri135) { testSingleMap("gri135"); }
TEST_F(SokobanMapTest, testgri136) { testSingleMap("gri136"); }
TEST_F(SokobanMapTest, testgri137) { testSingleMap("gri137"); }
TEST_F(SokobanMapTest, testgri138) { testSingleMap("gri138"); }
TEST_F(SokobanMapTest, testgri139) { testSingleMap("gri139"); }
TEST_F(SokobanMapTest, testgri13) { testSingleMap("gri13"); }
TEST_F(SokobanMapTest, testgri140) { testSingleMap("gri140"); }
TEST_F(SokobanMapTest, testgri14) { testSingleMap("gri14"); }
TEST_F(SokobanMapTest, testgri15) { testSingleMap("gri15"); }
TEST_F(SokobanMapTest, testgri16) { testSingleMap("gri16"); }
TEST_F(SokobanMapTest, testgri17) { testSingleMap("gri17"); }
TEST_F(SokobanMapTest, testgri18) { testSingleMap("gri18"); }
TEST_F(SokobanMapTest, testgri19) { testSingleMap("gri19"); }
TEST_F(SokobanMapTest, testgri20) { testSingleMap("gri20"); }
TEST_F(SokobanMapTest, testgri21) { testSingleMap("gri21"); }
TEST_F(SokobanMapTest, testgri22) { testSingleMap("gri22"); }
TEST_F(SokobanMapTest, testgri23) { testSingleMap("gri23"); }
TEST_F(SokobanMapTest, testgri24) { testSingleMap("gri24"); }
TEST_F(SokobanMapTest, testgri25) { testSingleMap("gri25"); }
TEST_F(SokobanMapTest, testgri26) { testSingleMap("gri26"); }
TEST_F(SokobanMapTest, testgri27) { testSingleMap("gri27"); }
TEST_F(SokobanMapTest, testgri28) { testSingleMap("gri28"); }
TEST_F(SokobanMapTest, testgri29) { testSingleMap("gri29"); }
TEST_F(SokobanMapTest, testgri30) { testSingleMap("gri30"); }
TEST_F(SokobanMapTest, testgri31) { testSingleMap("gri31"); }
TEST_F(SokobanMapTest, testgri32) { testSingleMap("gri32"); }
TEST_F(SokobanMapTest, testgri33) { testSingleMap("gri33"); }
TEST_F(SokobanMapTest, testgri34) { testSingleMap("gri34"); }
TEST_F(SokobanMapTest, testgri35) { testSingleMap("gri35"); }
TEST_F(SokobanMapTest, testgri36) { testSingleMap("gri36"); }
TEST_F(SokobanMapTest, testgri37) { testSingleMap("gri37"); }
TEST_F(SokobanMapTest, testgri38) { testSingleMap("gri38"); }
TEST_F(SokobanMapTest, testgri39) { testSingleMap("gri39"); }
TEST_F(SokobanMapTest, testgri40) { testSingleMap("gri40"); }
TEST_F(SokobanMapTest, testgri41) { testSingleMap("gri41"); }
TEST_F(SokobanMapTest, testgri42) { testSingleMap("gri42"); }
TEST_F(SokobanMapTest, testgri43) { testSingleMap("gri43"); }
TEST_F(SokobanMapTest, testgri44) { testSingleMap("gri44"); }
TEST_F(SokobanMapTest, testgri45) { testSingleMap("gri45"); }
TEST_F(SokobanMapTest, testgri46) { testSingleMap("gri46"); }
TEST_F(SokobanMapTest, testgri47) { testSingleMap("gri47"); }
TEST_F(SokobanMapTest, testgri48) { testSingleMap("gri48"); }
TEST_F(SokobanMapTest, testgri49) { testSingleMap("gri49"); }
TEST_F(SokobanMapTest, testgri50) { testSingleMap("gri50"); }
TEST_F(SokobanMapTest, testgri51) { testSingleMap("gri51"); }
TEST_F(SokobanMapTest, testgri52) { testSingleMap("gri52"); }
TEST_F(SokobanMapTest, testgri53) { testSingleMap("gri53"); }
TEST_F(SokobanMapTest, testgri54) { testSingleMap("gri54"); }
TEST_F(SokobanMapTest, testgri55) { testSingleMap("gri55"); }
TEST_F(SokobanMapTest, testgri56) { testSingleMap("gri56"); }
TEST_F(SokobanMapTest, testgri57) { testSingleMap("gri57"); }
TEST_F(SokobanMapTest, testgri58) { testSingleMap("gri58"); }
TEST_F(SokobanMapTest, testgri59) { testSingleMap("gri59"); }
TEST_F(SokobanMapTest, testgri60) { testSingleMap("gri60"); }
TEST_F(SokobanMapTest, testgri61) { testSingleMap("gri61"); }
TEST_F(SokobanMapTest, testgri62) { testSingleMap("gri62"); }
TEST_F(SokobanMapTest, testgri63) { testSingleMap("gri63"); }
TEST_F(SokobanMapTest, testgri64) { testSingleMap("gri64"); }
TEST_F(SokobanMapTest, testgri65) { testSingleMap("gri65"); }
TEST_F(SokobanMapTest, testgri66) { testSingleMap("gri66"); }
TEST_F(SokobanMapTest, testgri67) { testSingleMap("gri67"); }
TEST_F(SokobanMapTest, testgri68) { testSingleMap("gri68"); }
TEST_F(SokobanMapTest, testgri69) { testSingleMap("gri69"); }
TEST_F(SokobanMapTest, testgri70) { testSingleMap("gri70"); }
TEST_F(SokobanMapTest, testgri71) { testSingleMap("gri71"); }
TEST_F(SokobanMapTest, testgri72) { testSingleMap("gri72"); }
TEST_F(SokobanMapTest, testgri73) { testSingleMap("gri73"); }
TEST_F(SokobanMapTest, testgri74) { testSingleMap("gri74"); }
TEST_F(SokobanMapTest, testgri75) { testSingleMap("gri75"); }
TEST_F(SokobanMapTest, testgri76) { testSingleMap("gri76"); }
TEST_F(SokobanMapTest, testgri77) { testSingleMap("gri77"); }
TEST_F(SokobanMapTest, testgri78) { testSingleMap("gri78"); }
TEST_F(SokobanMapTest, testgri79) { testSingleMap("gri79"); }
TEST_F(SokobanMapTest, testgri80) { testSingleMap("gri80"); }
TEST_F(SokobanMapTest, testgri81) { testSingleMap("gri81"); }
TEST_F(SokobanMapTest, testgri82) { testSingleMap("gri82"); }
TEST_F(SokobanMapTest, testgri83) { testSingleMap("gri83"); }
TEST_F(SokobanMapTest, testgri84) { testSingleMap("gri84"); }
TEST_F(SokobanMapTest, testgri85) { testSingleMap("gri85"); }
TEST_F(SokobanMapTest, testgri86) { testSingleMap("gri86"); }
TEST_F(SokobanMapTest, testgri87) { testSingleMap("gri87"); }
TEST_F(SokobanMapTest, testgri88) { testSingleMap("gri88"); }
TEST_F(SokobanMapTest, testgri89) { testSingleMap("gri89"); }
TEST_F(SokobanMapTest, testgri90) { testSingleMap("gri90"); }
TEST_F(SokobanMapTest, testgri91) { testSingleMap("gri91"); }
TEST_F(SokobanMapTest, testgri92) { testSingleMap("gri92"); }
TEST_F(SokobanMapTest, testgri93) { testSingleMap("gri93"); }
TEST_F(SokobanMapTest, testgri94) { testSingleMap("gri94"); }
TEST_F(SokobanMapTest, testgri95) { testSingleMap("gri95"); }
TEST_F(SokobanMapTest, testgri96) { testSingleMap("gri96"); }
TEST_F(SokobanMapTest, testgri97) { testSingleMap("gri97"); }
TEST_F(SokobanMapTest, testgri98) { testSingleMap("gri98"); }
TEST_F(SokobanMapTest, testgri99) { testSingleMap("gri99"); }
TEST_F(SokobanMapTest, testgrigrcomet01) { testSingleMap("grigrcomet01"); }
TEST_F(SokobanMapTest, testgrigrcomet02) { testSingleMap("grigrcomet02"); }
TEST_F(SokobanMapTest, testgrigrcomet03) { testSingleMap("grigrcomet03"); }
TEST_F(SokobanMapTest, testgrigrcomet04) { testSingleMap("grigrcomet04"); }
TEST_F(SokobanMapTest, testgrigrcomet05) { testSingleMap("grigrcomet05"); }
TEST_F(SokobanMapTest, testgrigrcomet06) { testSingleMap("grigrcomet06"); }
TEST_F(SokobanMapTest, testgrigrcomet07) { testSingleMap("grigrcomet07"); }
TEST_F(SokobanMapTest, testgrigrcomet08) { testSingleMap("grigrcomet08"); }
TEST_F(SokobanMapTest, testgrigrcomet09) { testSingleMap("grigrcomet09"); }
TEST_F(SokobanMapTest, testgrigrcomet10) { testSingleMap("grigrcomet10"); }
TEST_F(SokobanMapTest, testgrigrcomet11) { testSingleMap("grigrcomet11"); }
TEST_F(SokobanMapTest, testgrigrcomet12) { testSingleMap("grigrcomet12"); }
TEST_F(SokobanMapTest, testgrigrcomet13) { testSingleMap("grigrcomet13"); }
TEST_F(SokobanMapTest, testgrigrcomet14) { testSingleMap("grigrcomet14"); }
TEST_F(SokobanMapTest, testgrigrcomet15) { testSingleMap("grigrcomet15"); }
TEST_F(SokobanMapTest, testgrigrcomet16) { testSingleMap("grigrcomet16"); }
TEST_F(SokobanMapTest, testgrigrcomet17) { testSingleMap("grigrcomet17"); }
TEST_F(SokobanMapTest, testgrigrcomet18) { testSingleMap("grigrcomet18"); }
TEST_F(SokobanMapTest, testgrigrcomet19) { testSingleMap("grigrcomet19"); }
TEST_F(SokobanMapTest, testgrigrcomet20) { testSingleMap("grigrcomet20"); }
TEST_F(SokobanMapTest, testgrigrcomet21) { testSingleMap("grigrcomet21"); }
TEST_F(SokobanMapTest, testgrigrcomet22) { testSingleMap("grigrcomet22"); }
TEST_F(SokobanMapTest, testgrigrcomet23) { testSingleMap("grigrcomet23"); }
TEST_F(SokobanMapTest, testgrigrcomet24) { testSingleMap("grigrcomet24"); }
TEST_F(SokobanMapTest, testgrigrcomet25) { testSingleMap("grigrcomet25"); }
TEST_F(SokobanMapTest, testgrigrcomet26) { testSingleMap("grigrcomet26"); }
TEST_F(SokobanMapTest, testgrigrcomet27) { testSingleMap("grigrcomet27"); }
TEST_F(SokobanMapTest, testgrigrcomet28) { testSingleMap("grigrcomet28"); }
TEST_F(SokobanMapTest, testgrigrcomet29) { testSingleMap("grigrcomet29"); }
TEST_F(SokobanMapTest, testgrigrcomet30) { testSingleMap("grigrcomet30"); }
TEST_F(SokobanMapTest, testgrigrspecial01) { testSingleMap("grigrspecial01"); }
TEST_F(SokobanMapTest, testgrigrspecial02) { testSingleMap("grigrspecial02"); }
TEST_F(SokobanMapTest, testgrigrspecial03) { testSingleMap("grigrspecial03"); }
TEST_F(SokobanMapTest, testgrigrspecial04) { testSingleMap("grigrspecial04"); }
TEST_F(SokobanMapTest, testgrigrspecial05) { testSingleMap("grigrspecial05"); }
TEST_F(SokobanMapTest, testgrigrspecial06) { testSingleMap("grigrspecial06"); }
TEST_F(SokobanMapTest, testgrigrspecial07) { testSingleMap("grigrspecial07"); }
TEST_F(SokobanMapTest, testgrigrspecial08) { testSingleMap("grigrspecial08"); }
TEST_F(SokobanMapTest, testgrigrspecial09) { testSingleMap("grigrspecial09"); }
TEST_F(SokobanMapTest, testgrigrspecial10) { testSingleMap("grigrspecial10"); }
TEST_F(SokobanMapTest, testgrigrspecial11) { testSingleMap("grigrspecial11"); }
TEST_F(SokobanMapTest, testgrigrspecial12) { testSingleMap("grigrspecial12"); }
TEST_F(SokobanMapTest, testgrigrspecial13) { testSingleMap("grigrspecial13"); }
TEST_F(SokobanMapTest, testgrigrspecial14) { testSingleMap("grigrspecial14"); }
TEST_F(SokobanMapTest, testgrigrspecial15) { testSingleMap("grigrspecial15"); }
TEST_F(SokobanMapTest, testgrigrspecial16) { testSingleMap("grigrspecial16"); }
TEST_F(SokobanMapTest, testgrigrspecial17) { testSingleMap("grigrspecial17"); }
TEST_F(SokobanMapTest, testgrigrspecial18) { testSingleMap("grigrspecial18"); }
TEST_F(SokobanMapTest, testgrigrspecial19) { testSingleMap("grigrspecial19"); }
TEST_F(SokobanMapTest, testgrigrspecial20) { testSingleMap("grigrspecial20"); }
TEST_F(SokobanMapTest, testgrigrspecial21) { testSingleMap("grigrspecial21"); }
TEST_F(SokobanMapTest, testgrigrspecial22) { testSingleMap("grigrspecial22"); }
TEST_F(SokobanMapTest, testgrigrspecial23) { testSingleMap("grigrspecial23"); }
TEST_F(SokobanMapTest, testgrigrspecial24) { testSingleMap("grigrspecial24"); }
TEST_F(SokobanMapTest, testgrigrspecial25) { testSingleMap("grigrspecial25"); }
TEST_F(SokobanMapTest, testgrigrspecial26) { testSingleMap("grigrspecial26"); }
TEST_F(SokobanMapTest, testgrigrspecial27) { testSingleMap("grigrspecial27"); }
TEST_F(SokobanMapTest, testgrigrspecial28) { testSingleMap("grigrspecial28"); }
TEST_F(SokobanMapTest, testgrigrspecial29) { testSingleMap("grigrspecial29"); }
TEST_F(SokobanMapTest, testgrigrspecial30) { testSingleMap("grigrspecial30"); }
TEST_F(SokobanMapTest, testgrigrspecial31) { testSingleMap("grigrspecial31"); }
TEST_F(SokobanMapTest, testgrigrspecial32) { testSingleMap("grigrspecial32"); }
TEST_F(SokobanMapTest, testgrigrspecial33) { testSingleMap("grigrspecial33"); }
TEST_F(SokobanMapTest, testgrigrspecial34) { testSingleMap("grigrspecial34"); }
TEST_F(SokobanMapTest, testgrigrspecial35) { testSingleMap("grigrspecial35"); }
TEST_F(SokobanMapTest, testgrigrspecial36) { testSingleMap("grigrspecial36"); }
TEST_F(SokobanMapTest, testgrigrspecial37) { testSingleMap("grigrspecial37"); }
TEST_F(SokobanMapTest, testgrigrspecial38) { testSingleMap("grigrspecial38"); }
TEST_F(SokobanMapTest, testgrigrspecial39) { testSingleMap("grigrspecial39"); }
TEST_F(SokobanMapTest, testgrigrspecial40) { testSingleMap("grigrspecial40"); }
TEST_F(SokobanMapTest, testgrigrstar01) { testSingleMap("grigrstar01"); }
TEST_F(SokobanMapTest, testgrigrstar02) { testSingleMap("grigrstar02"); }
TEST_F(SokobanMapTest, testgrigrstar03) { testSingleMap("grigrstar03"); }
TEST_F(SokobanMapTest, testgrigrstar04) { testSingleMap("grigrstar04"); }
TEST_F(SokobanMapTest, testgrigrstar05) { testSingleMap("grigrstar05"); }
TEST_F(SokobanMapTest, testgrigrstar06) { testSingleMap("grigrstar06"); }
TEST_F(SokobanMapTest, testgrigrstar07) { testSingleMap("grigrstar07"); }
TEST_F(SokobanMapTest, testgrigrstar08) { testSingleMap("grigrstar08"); }
TEST_F(SokobanMapTest, testgrigrstar09) { testSingleMap("grigrstar09"); }
TEST_F(SokobanMapTest, testgrigrstar10) { testSingleMap("grigrstar10"); }
TEST_F(SokobanMapTest, testgrigrstar11) { testSingleMap("grigrstar11"); }
TEST_F(SokobanMapTest, testgrigrstar12) { testSingleMap("grigrstar12"); }
TEST_F(SokobanMapTest, testgrigrstar13) { testSingleMap("grigrstar13"); }
TEST_F(SokobanMapTest, testgrigrstar14) { testSingleMap("grigrstar14"); }
TEST_F(SokobanMapTest, testgrigrstar15) { testSingleMap("grigrstar15"); }
TEST_F(SokobanMapTest, testgrigrstar16) { testSingleMap("grigrstar16"); }
TEST_F(SokobanMapTest, testgrigrstar17) { testSingleMap("grigrstar17"); }
TEST_F(SokobanMapTest, testgrigrstar18) { testSingleMap("grigrstar18"); }
TEST_F(SokobanMapTest, testgrigrstar19) { testSingleMap("grigrstar19"); }
TEST_F(SokobanMapTest, testgrigrstar20) { testSingleMap("grigrstar20"); }
TEST_F(SokobanMapTest, testgrigrstar21) { testSingleMap("grigrstar21"); }
TEST_F(SokobanMapTest, testgrigrstar22) { testSingleMap("grigrstar22"); }
TEST_F(SokobanMapTest, testgrigrstar23) { testSingleMap("grigrstar23"); }
TEST_F(SokobanMapTest, testgrigrstar24) { testSingleMap("grigrstar24"); }
TEST_F(SokobanMapTest, testgrigrstar25) { testSingleMap("grigrstar25"); }
TEST_F(SokobanMapTest, testgrigrstar26) { testSingleMap("grigrstar26"); }
TEST_F(SokobanMapTest, testgrigrstar27) { testSingleMap("grigrstar27"); }
TEST_F(SokobanMapTest, testgrigrstar28) { testSingleMap("grigrstar28"); }
TEST_F(SokobanMapTest, testgrigrstar29) { testSingleMap("grigrstar29"); }
TEST_F(SokobanMapTest, testgrigrstar30) { testSingleMap("grigrstar30"); }
TEST_F(SokobanMapTest, testgrigrsun01) { testSingleMap("grigrsun01"); }
TEST_F(SokobanMapTest, testgrigrsun02) { testSingleMap("grigrsun02"); }
TEST_F(SokobanMapTest, testgrigrsun03) { testSingleMap("grigrsun03"); }
TEST_F(SokobanMapTest, testgrigrsun04) { testSingleMap("grigrsun04"); }
TEST_F(SokobanMapTest, testgrigrsun05) { testSingleMap("grigrsun05"); }
TEST_F(SokobanMapTest, testgrigrsun06) { testSingleMap("grigrsun06"); }
TEST_F(SokobanMapTest, testgrigrsun07) { testSingleMap("grigrsun07"); }
TEST_F(SokobanMapTest, testgrigrsun08) { testSingleMap("grigrsun08"); }
TEST_F(SokobanMapTest, testgrigrsun09) { testSingleMap("grigrsun09"); }
TEST_F(SokobanMapTest, testgrigrsun10) { testSingleMap("grigrsun10"); }
