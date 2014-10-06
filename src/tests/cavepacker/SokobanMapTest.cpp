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

	void testMapRegion (int begin, const std::string& prefix, int amount = 10)
	{
		NetworkTestListener listener;
		NetworkTestServerListener serverListener;
		ASSERT_TRUE(_serviceProvider.getNetwork().openServer(12345, &serverListener)) << "Failed to open the server";
		ASSERT_TRUE(_serviceProvider.getNetwork().openClient("localhost", 12345, &listener)) << "Failed to open the client";

		for (int i = begin; i < begin + amount; ++i) {
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

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps10)
{
	testMapRegion(1, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps20)
{
	testMapRegion(11, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps30)
{
	testMapRegion(21, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps40)
{
	testMapRegion(31, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps50)
{
	testMapRegion(41, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps60)
{
	testMapRegion(51, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps70)
{
	testMapRegion(61, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps80)
{
	testMapRegion(71, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testXSokobanMaps90)
{
	testMapRegion(81, "xsokoban");
}

TEST_F(SokobanMapTest, DISABLED_testKSokobanMaps)
{
	testMapRegion(0, "ksokoban", 354);
}

TEST_F(SokobanMapTest, DISABLED_testGri)
{
	testMapRegion(1, "gri", 140);
	testMapRegion(1, "grigrcomet", 30);
	testMapRegion(0, "grigrspecial", 40);
	testMapRegion(0, "grigrstar", 30);
	testMapRegion(0, "grigrstar", 10);
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
TEST_F(SokobanMapTest, testKSokoban0000) { testSingleMap("ksokoban0000"); }
TEST_F(SokobanMapTest, testKSokoban0001) { testSingleMap("ksokoban0001"); }
TEST_F(SokobanMapTest, testKSokoban0002) { testSingleMap("ksokoban0002"); }
TEST_F(SokobanMapTest, testKSokoban0003) { testSingleMap("ksokoban0003"); }
TEST_F(SokobanMapTest, testKSokoban0004) { testSingleMap("ksokoban0004"); }
TEST_F(SokobanMapTest, testKSokoban0005) { testSingleMap("ksokoban0005"); }
TEST_F(SokobanMapTest, testKSokoban0006) { testSingleMap("ksokoban0006"); }
TEST_F(SokobanMapTest, testKSokoban0007) { testSingleMap("ksokoban0007"); }
TEST_F(SokobanMapTest, testKSokoban0008) { testSingleMap("ksokoban0008"); }
TEST_F(SokobanMapTest, testKSokoban0009) { testSingleMap("ksokoban0009"); }
TEST_F(SokobanMapTest, testKSokoban0010) { testSingleMap("ksokoban0010"); }
TEST_F(SokobanMapTest, testKSokoban0011) { testSingleMap("ksokoban0011"); }
TEST_F(SokobanMapTest, testKSokoban0012) { testSingleMap("ksokoban0012"); }
TEST_F(SokobanMapTest, testKSokoban0013) { testSingleMap("ksokoban0013"); }
TEST_F(SokobanMapTest, testKSokoban0014) { testSingleMap("ksokoban0014"); }
TEST_F(SokobanMapTest, testKSokoban0015) { testSingleMap("ksokoban0015"); }
TEST_F(SokobanMapTest, testKSokoban0016) { testSingleMap("ksokoban0016"); }
TEST_F(SokobanMapTest, testKSokoban0017) { testSingleMap("ksokoban0017"); }
TEST_F(SokobanMapTest, testKSokoban0018) { testSingleMap("ksokoban0018"); }
TEST_F(SokobanMapTest, testKSokoban0019) { testSingleMap("ksokoban0019"); }
TEST_F(SokobanMapTest, testKSokoban0020) { testSingleMap("ksokoban0020"); }
TEST_F(SokobanMapTest, testKSokoban0021) { testSingleMap("ksokoban0021"); }
TEST_F(SokobanMapTest, testKSokoban0022) { testSingleMap("ksokoban0022"); }
TEST_F(SokobanMapTest, testKSokoban0023) { testSingleMap("ksokoban0023"); }
TEST_F(SokobanMapTest, testKSokoban0024) { testSingleMap("ksokoban0024"); }
TEST_F(SokobanMapTest, testKSokoban0025) { testSingleMap("ksokoban0025"); }
TEST_F(SokobanMapTest, testKSokoban0026) { testSingleMap("ksokoban0026"); }
TEST_F(SokobanMapTest, testKSokoban0027) { testSingleMap("ksokoban0027"); }
TEST_F(SokobanMapTest, testKSokoban0028) { testSingleMap("ksokoban0028"); }
TEST_F(SokobanMapTest, testKSokoban0029) { testSingleMap("ksokoban0029"); }
TEST_F(SokobanMapTest, testKSokoban0030) { testSingleMap("ksokoban0030"); }
TEST_F(SokobanMapTest, testKSokoban0031) { testSingleMap("ksokoban0031"); }
TEST_F(SokobanMapTest, testKSokoban0032) { testSingleMap("ksokoban0032"); }
TEST_F(SokobanMapTest, testKSokoban0033) { testSingleMap("ksokoban0033"); }
TEST_F(SokobanMapTest, testKSokoban0034) { testSingleMap("ksokoban0034"); }
TEST_F(SokobanMapTest, testKSokoban0035) { testSingleMap("ksokoban0035"); }
TEST_F(SokobanMapTest, testKSokoban0036) { testSingleMap("ksokoban0036"); }
TEST_F(SokobanMapTest, testKSokoban0037) { testSingleMap("ksokoban0037"); }
TEST_F(SokobanMapTest, testKSokoban0038) { testSingleMap("ksokoban0038"); }
TEST_F(SokobanMapTest, testKSokoban0039) { testSingleMap("ksokoban0039"); }
TEST_F(SokobanMapTest, testKSokoban0040) { testSingleMap("ksokoban0040"); }
TEST_F(SokobanMapTest, testKSokoban0041) { testSingleMap("ksokoban0041"); }
TEST_F(SokobanMapTest, testKSokoban0042) { testSingleMap("ksokoban0042"); }
TEST_F(SokobanMapTest, testKSokoban0043) { testSingleMap("ksokoban0043"); }
TEST_F(SokobanMapTest, testKSokoban0044) { testSingleMap("ksokoban0044"); }
TEST_F(SokobanMapTest, testKSokoban0045) { testSingleMap("ksokoban0045"); }
TEST_F(SokobanMapTest, testKSokoban0046) { testSingleMap("ksokoban0046"); }
TEST_F(SokobanMapTest, testKSokoban0047) { testSingleMap("ksokoban0047"); }
TEST_F(SokobanMapTest, testKSokoban0048) { testSingleMap("ksokoban0048"); }
TEST_F(SokobanMapTest, testKSokoban0049) { testSingleMap("ksokoban0049"); }
TEST_F(SokobanMapTest, testKSokoban0050) { testSingleMap("ksokoban0050"); }
TEST_F(SokobanMapTest, testKSokoban0051) { testSingleMap("ksokoban0051"); }
TEST_F(SokobanMapTest, testKSokoban0052) { testSingleMap("ksokoban0052"); }
TEST_F(SokobanMapTest, testKSokoban0053) { testSingleMap("ksokoban0053"); }
TEST_F(SokobanMapTest, testKSokoban0054) { testSingleMap("ksokoban0054"); }
TEST_F(SokobanMapTest, testKSokoban0055) { testSingleMap("ksokoban0055"); }
TEST_F(SokobanMapTest, testKSokoban0056) { testSingleMap("ksokoban0056"); }
TEST_F(SokobanMapTest, testKSokoban0057) { testSingleMap("ksokoban0057"); }
TEST_F(SokobanMapTest, testKSokoban0058) { testSingleMap("ksokoban0058"); }
TEST_F(SokobanMapTest, testKSokoban0059) { testSingleMap("ksokoban0059"); }
TEST_F(SokobanMapTest, testKSokoban0060) { testSingleMap("ksokoban0060"); }
TEST_F(SokobanMapTest, testKSokoban0061) { testSingleMap("ksokoban0061"); }
TEST_F(SokobanMapTest, testKSokoban0062) { testSingleMap("ksokoban0062"); }
TEST_F(SokobanMapTest, testKSokoban0063) { testSingleMap("ksokoban0063"); }
TEST_F(SokobanMapTest, testKSokoban0064) { testSingleMap("ksokoban0064"); }
TEST_F(SokobanMapTest, testKSokoban0065) { testSingleMap("ksokoban0065"); }
TEST_F(SokobanMapTest, testKSokoban0066) { testSingleMap("ksokoban0066"); }
TEST_F(SokobanMapTest, testKSokoban0067) { testSingleMap("ksokoban0067"); }
TEST_F(SokobanMapTest, testKSokoban0068) { testSingleMap("ksokoban0068"); }
TEST_F(SokobanMapTest, testKSokoban0069) { testSingleMap("ksokoban0069"); }
TEST_F(SokobanMapTest, testKSokoban0070) { testSingleMap("ksokoban0070"); }
TEST_F(SokobanMapTest, testKSokoban0071) { testSingleMap("ksokoban0071"); }
TEST_F(SokobanMapTest, testKSokoban0072) { testSingleMap("ksokoban0072"); }
TEST_F(SokobanMapTest, testKSokoban0073) { testSingleMap("ksokoban0073"); }
TEST_F(SokobanMapTest, testKSokoban0074) { testSingleMap("ksokoban0074"); }
TEST_F(SokobanMapTest, testKSokoban0075) { testSingleMap("ksokoban0075"); }
TEST_F(SokobanMapTest, testKSokoban0076) { testSingleMap("ksokoban0076"); }
TEST_F(SokobanMapTest, testKSokoban0077) { testSingleMap("ksokoban0077"); }
TEST_F(SokobanMapTest, testKSokoban0078) { testSingleMap("ksokoban0078"); }
TEST_F(SokobanMapTest, testKSokoban0079) { testSingleMap("ksokoban0079"); }
TEST_F(SokobanMapTest, testKSokoban0080) { testSingleMap("ksokoban0080"); }
TEST_F(SokobanMapTest, testKSokoban0081) { testSingleMap("ksokoban0081"); }
TEST_F(SokobanMapTest, testKSokoban0082) { testSingleMap("ksokoban0082"); }
TEST_F(SokobanMapTest, testKSokoban0083) { testSingleMap("ksokoban0083"); }
TEST_F(SokobanMapTest, testKSokoban0084) { testSingleMap("ksokoban0084"); }
TEST_F(SokobanMapTest, testKSokoban0085) { testSingleMap("ksokoban0085"); }
TEST_F(SokobanMapTest, testKSokoban0086) { testSingleMap("ksokoban0086"); }
TEST_F(SokobanMapTest, testKSokoban0087) { testSingleMap("ksokoban0087"); }
TEST_F(SokobanMapTest, testKSokoban0088) { testSingleMap("ksokoban0088"); }
TEST_F(SokobanMapTest, testKSokoban0089) { testSingleMap("ksokoban0089"); }
TEST_F(SokobanMapTest, testKSokoban0090) { testSingleMap("ksokoban0090"); }
TEST_F(SokobanMapTest, testKSokoban0091) { testSingleMap("ksokoban0091"); }
TEST_F(SokobanMapTest, testKSokoban0092) { testSingleMap("ksokoban0092"); }
TEST_F(SokobanMapTest, testKSokoban0093) { testSingleMap("ksokoban0093"); }
TEST_F(SokobanMapTest, testKSokoban0094) { testSingleMap("ksokoban0094"); }
TEST_F(SokobanMapTest, testKSokoban0095) { testSingleMap("ksokoban0095"); }
TEST_F(SokobanMapTest, testKSokoban0096) { testSingleMap("ksokoban0096"); }
TEST_F(SokobanMapTest, testKSokoban0097) { testSingleMap("ksokoban0097"); }
TEST_F(SokobanMapTest, testKSokoban0098) { testSingleMap("ksokoban0098"); }
TEST_F(SokobanMapTest, testKSokoban0099) { testSingleMap("ksokoban0099"); }
TEST_F(SokobanMapTest, testKSokoban0100) { testSingleMap("ksokoban0100"); }
TEST_F(SokobanMapTest, testKSokoban0101) { testSingleMap("ksokoban0101"); }
TEST_F(SokobanMapTest, testKSokoban0102) { testSingleMap("ksokoban0102"); }
TEST_F(SokobanMapTest, testKSokoban0103) { testSingleMap("ksokoban0103"); }
TEST_F(SokobanMapTest, testKSokoban0104) { testSingleMap("ksokoban0104"); }
TEST_F(SokobanMapTest, testKSokoban0105) { testSingleMap("ksokoban0105"); }
TEST_F(SokobanMapTest, testKSokoban0106) { testSingleMap("ksokoban0106"); }
TEST_F(SokobanMapTest, testKSokoban0107) { testSingleMap("ksokoban0107"); }
TEST_F(SokobanMapTest, testKSokoban0108) { testSingleMap("ksokoban0108"); }
TEST_F(SokobanMapTest, testKSokoban0109) { testSingleMap("ksokoban0109"); }
TEST_F(SokobanMapTest, testKSokoban0110) { testSingleMap("ksokoban0110"); }
TEST_F(SokobanMapTest, testKSokoban0111) { testSingleMap("ksokoban0111"); }
TEST_F(SokobanMapTest, testKSokoban0112) { testSingleMap("ksokoban0112"); }
TEST_F(SokobanMapTest, testKSokoban0113) { testSingleMap("ksokoban0113"); }
TEST_F(SokobanMapTest, testKSokoban0114) { testSingleMap("ksokoban0114"); }
TEST_F(SokobanMapTest, testKSokoban0115) { testSingleMap("ksokoban0115"); }
TEST_F(SokobanMapTest, testKSokoban0116) { testSingleMap("ksokoban0116"); }
TEST_F(SokobanMapTest, testKSokoban0117) { testSingleMap("ksokoban0117"); }
TEST_F(SokobanMapTest, testKSokoban0118) { testSingleMap("ksokoban0118"); }
TEST_F(SokobanMapTest, testKSokoban0119) { testSingleMap("ksokoban0119"); }
TEST_F(SokobanMapTest, testKSokoban0120) { testSingleMap("ksokoban0120"); }
TEST_F(SokobanMapTest, testKSokoban0121) { testSingleMap("ksokoban0121"); }
TEST_F(SokobanMapTest, testKSokoban0122) { testSingleMap("ksokoban0122"); }
TEST_F(SokobanMapTest, testKSokoban0123) { testSingleMap("ksokoban0123"); }
TEST_F(SokobanMapTest, testKSokoban0124) { testSingleMap("ksokoban0124"); }
TEST_F(SokobanMapTest, testKSokoban0125) { testSingleMap("ksokoban0125"); }
TEST_F(SokobanMapTest, testKSokoban0126) { testSingleMap("ksokoban0126"); }
TEST_F(SokobanMapTest, testKSokoban0127) { testSingleMap("ksokoban0127"); }
TEST_F(SokobanMapTest, testKSokoban0128) { testSingleMap("ksokoban0128"); }
TEST_F(SokobanMapTest, testKSokoban0129) { testSingleMap("ksokoban0129"); }
TEST_F(SokobanMapTest, testKSokoban0130) { testSingleMap("ksokoban0130"); }
TEST_F(SokobanMapTest, testKSokoban0131) { testSingleMap("ksokoban0131"); }
TEST_F(SokobanMapTest, testKSokoban0132) { testSingleMap("ksokoban0132"); }
TEST_F(SokobanMapTest, testKSokoban0133) { testSingleMap("ksokoban0133"); }
TEST_F(SokobanMapTest, testKSokoban0134) { testSingleMap("ksokoban0134"); }
TEST_F(SokobanMapTest, testKSokoban0135) { testSingleMap("ksokoban0135"); }
TEST_F(SokobanMapTest, testKSokoban0136) { testSingleMap("ksokoban0136"); }
TEST_F(SokobanMapTest, testKSokoban0137) { testSingleMap("ksokoban0137"); }
TEST_F(SokobanMapTest, testKSokoban0138) { testSingleMap("ksokoban0138"); }
TEST_F(SokobanMapTest, testKSokoban0139) { testSingleMap("ksokoban0139"); }
TEST_F(SokobanMapTest, testKSokoban0140) { testSingleMap("ksokoban0140"); }
TEST_F(SokobanMapTest, testKSokoban0141) { testSingleMap("ksokoban0141"); }
TEST_F(SokobanMapTest, testKSokoban0142) { testSingleMap("ksokoban0142"); }
TEST_F(SokobanMapTest, testKSokoban0143) { testSingleMap("ksokoban0143"); }
TEST_F(SokobanMapTest, testKSokoban0144) { testSingleMap("ksokoban0144"); }
TEST_F(SokobanMapTest, testKSokoban0145) { testSingleMap("ksokoban0145"); }
TEST_F(SokobanMapTest, testKSokoban0146) { testSingleMap("ksokoban0146"); }
TEST_F(SokobanMapTest, testKSokoban0147) { testSingleMap("ksokoban0147"); }
TEST_F(SokobanMapTest, testKSokoban0148) { testSingleMap("ksokoban0148"); }
TEST_F(SokobanMapTest, testKSokoban0149) { testSingleMap("ksokoban0149"); }
TEST_F(SokobanMapTest, testKSokoban0150) { testSingleMap("ksokoban0150"); }
TEST_F(SokobanMapTest, testKSokoban0151) { testSingleMap("ksokoban0151"); }
TEST_F(SokobanMapTest, testKSokoban0152) { testSingleMap("ksokoban0152"); }
TEST_F(SokobanMapTest, testKSokoban0153) { testSingleMap("ksokoban0153"); }
TEST_F(SokobanMapTest, testKSokoban0154) { testSingleMap("ksokoban0154"); }
TEST_F(SokobanMapTest, testKSokoban0155) { testSingleMap("ksokoban0155"); }
TEST_F(SokobanMapTest, testKSokoban0156) { testSingleMap("ksokoban0156"); }
TEST_F(SokobanMapTest, testKSokoban0157) { testSingleMap("ksokoban0157"); }
TEST_F(SokobanMapTest, testKSokoban0158) { testSingleMap("ksokoban0158"); }
TEST_F(SokobanMapTest, testKSokoban0159) { testSingleMap("ksokoban0159"); }
TEST_F(SokobanMapTest, testKSokoban0160) { testSingleMap("ksokoban0160"); }
TEST_F(SokobanMapTest, testKSokoban0161) { testSingleMap("ksokoban0161"); }
TEST_F(SokobanMapTest, testKSokoban0162) { testSingleMap("ksokoban0162"); }
TEST_F(SokobanMapTest, testKSokoban0163) { testSingleMap("ksokoban0163"); }
TEST_F(SokobanMapTest, testKSokoban0164) { testSingleMap("ksokoban0164"); }
TEST_F(SokobanMapTest, testKSokoban0165) { testSingleMap("ksokoban0165"); }
TEST_F(SokobanMapTest, testKSokoban0166) { testSingleMap("ksokoban0166"); }
TEST_F(SokobanMapTest, testKSokoban0167) { testSingleMap("ksokoban0167"); }
TEST_F(SokobanMapTest, testKSokoban0168) { testSingleMap("ksokoban0168"); }
TEST_F(SokobanMapTest, testKSokoban0169) { testSingleMap("ksokoban0169"); }
TEST_F(SokobanMapTest, testKSokoban0170) { testSingleMap("ksokoban0170"); }
TEST_F(SokobanMapTest, testKSokoban0171) { testSingleMap("ksokoban0171"); }
TEST_F(SokobanMapTest, testKSokoban0172) { testSingleMap("ksokoban0172"); }
TEST_F(SokobanMapTest, testKSokoban0173) { testSingleMap("ksokoban0173"); }
TEST_F(SokobanMapTest, testKSokoban0174) { testSingleMap("ksokoban0174"); }
TEST_F(SokobanMapTest, testKSokoban0175) { testSingleMap("ksokoban0175"); }
TEST_F(SokobanMapTest, testKSokoban0176) { testSingleMap("ksokoban0176"); }
TEST_F(SokobanMapTest, testKSokoban0177) { testSingleMap("ksokoban0177"); }
TEST_F(SokobanMapTest, testKSokoban0178) { testSingleMap("ksokoban0178"); }
TEST_F(SokobanMapTest, testKSokoban0179) { testSingleMap("ksokoban0179"); }
TEST_F(SokobanMapTest, testKSokoban0180) { testSingleMap("ksokoban0180"); }
TEST_F(SokobanMapTest, testKSokoban0181) { testSingleMap("ksokoban0181"); }
TEST_F(SokobanMapTest, testKSokoban0182) { testSingleMap("ksokoban0182"); }
TEST_F(SokobanMapTest, testKSokoban0183) { testSingleMap("ksokoban0183"); }
TEST_F(SokobanMapTest, testKSokoban0184) { testSingleMap("ksokoban0184"); }
TEST_F(SokobanMapTest, testKSokoban0185) { testSingleMap("ksokoban0185"); }
TEST_F(SokobanMapTest, testKSokoban0186) { testSingleMap("ksokoban0186"); }
TEST_F(SokobanMapTest, testKSokoban0187) { testSingleMap("ksokoban0187"); }
TEST_F(SokobanMapTest, testKSokoban0188) { testSingleMap("ksokoban0188"); }
TEST_F(SokobanMapTest, testKSokoban0189) { testSingleMap("ksokoban0189"); }
TEST_F(SokobanMapTest, testKSokoban0190) { testSingleMap("ksokoban0190"); }
TEST_F(SokobanMapTest, testKSokoban0191) { testSingleMap("ksokoban0191"); }
TEST_F(SokobanMapTest, testKSokoban0192) { testSingleMap("ksokoban0192"); }
TEST_F(SokobanMapTest, testKSokoban0193) { testSingleMap("ksokoban0193"); }
TEST_F(SokobanMapTest, testKSokoban0194) { testSingleMap("ksokoban0194"); }
TEST_F(SokobanMapTest, testKSokoban0195) { testSingleMap("ksokoban0195"); }
TEST_F(SokobanMapTest, testKSokoban0196) { testSingleMap("ksokoban0196"); }
TEST_F(SokobanMapTest, testKSokoban0197) { testSingleMap("ksokoban0197"); }
TEST_F(SokobanMapTest, testKSokoban0198) { testSingleMap("ksokoban0198"); }
TEST_F(SokobanMapTest, testKSokoban0199) { testSingleMap("ksokoban0199"); }
TEST_F(SokobanMapTest, testKSokoban0200) { testSingleMap("ksokoban0200"); }
TEST_F(SokobanMapTest, testKSokoban0201) { testSingleMap("ksokoban0201"); }
TEST_F(SokobanMapTest, testKSokoban0202) { testSingleMap("ksokoban0202"); }
TEST_F(SokobanMapTest, testKSokoban0203) { testSingleMap("ksokoban0203"); }
TEST_F(SokobanMapTest, testKSokoban0204) { testSingleMap("ksokoban0204"); }
TEST_F(SokobanMapTest, testKSokoban0205) { testSingleMap("ksokoban0205"); }
TEST_F(SokobanMapTest, testKSokoban0206) { testSingleMap("ksokoban0206"); }
TEST_F(SokobanMapTest, testKSokoban0207) { testSingleMap("ksokoban0207"); }
TEST_F(SokobanMapTest, testKSokoban0208) { testSingleMap("ksokoban0208"); }
TEST_F(SokobanMapTest, testKSokoban0209) { testSingleMap("ksokoban0209"); }
TEST_F(SokobanMapTest, testKSokoban0210) { testSingleMap("ksokoban0210"); }
TEST_F(SokobanMapTest, testKSokoban0211) { testSingleMap("ksokoban0211"); }
TEST_F(SokobanMapTest, testKSokoban0212) { testSingleMap("ksokoban0212"); }
TEST_F(SokobanMapTest, testKSokoban0213) { testSingleMap("ksokoban0213"); }
TEST_F(SokobanMapTest, testKSokoban0214) { testSingleMap("ksokoban0214"); }
TEST_F(SokobanMapTest, testKSokoban0215) { testSingleMap("ksokoban0215"); }
TEST_F(SokobanMapTest, testKSokoban0216) { testSingleMap("ksokoban0216"); }
TEST_F(SokobanMapTest, testKSokoban0217) { testSingleMap("ksokoban0217"); }
TEST_F(SokobanMapTest, testKSokoban0218) { testSingleMap("ksokoban0218"); }
TEST_F(SokobanMapTest, testKSokoban0219) { testSingleMap("ksokoban0219"); }
TEST_F(SokobanMapTest, testKSokoban0220) { testSingleMap("ksokoban0220"); }
TEST_F(SokobanMapTest, testKSokoban0221) { testSingleMap("ksokoban0221"); }
TEST_F(SokobanMapTest, testKSokoban0222) { testSingleMap("ksokoban0222"); }
TEST_F(SokobanMapTest, testKSokoban0223) { testSingleMap("ksokoban0223"); }
TEST_F(SokobanMapTest, testKSokoban0224) { testSingleMap("ksokoban0224"); }
TEST_F(SokobanMapTest, testKSokoban0225) { testSingleMap("ksokoban0225"); }
TEST_F(SokobanMapTest, testKSokoban0226) { testSingleMap("ksokoban0226"); }
TEST_F(SokobanMapTest, testKSokoban0227) { testSingleMap("ksokoban0227"); }
TEST_F(SokobanMapTest, testKSokoban0228) { testSingleMap("ksokoban0228"); }
TEST_F(SokobanMapTest, testKSokoban0229) { testSingleMap("ksokoban0229"); }
TEST_F(SokobanMapTest, testKSokoban0230) { testSingleMap("ksokoban0230"); }
TEST_F(SokobanMapTest, testKSokoban0231) { testSingleMap("ksokoban0231"); }
TEST_F(SokobanMapTest, testKSokoban0232) { testSingleMap("ksokoban0232"); }
TEST_F(SokobanMapTest, testKSokoban0233) { testSingleMap("ksokoban0233"); }
TEST_F(SokobanMapTest, testKSokoban0234) { testSingleMap("ksokoban0234"); }
TEST_F(SokobanMapTest, testKSokoban0235) { testSingleMap("ksokoban0235"); }
TEST_F(SokobanMapTest, testKSokoban0236) { testSingleMap("ksokoban0236"); }
TEST_F(SokobanMapTest, testKSokoban0237) { testSingleMap("ksokoban0237"); }
TEST_F(SokobanMapTest, testKSokoban0238) { testSingleMap("ksokoban0238"); }
TEST_F(SokobanMapTest, testKSokoban0239) { testSingleMap("ksokoban0239"); }
TEST_F(SokobanMapTest, testKSokoban0240) { testSingleMap("ksokoban0240"); }
TEST_F(SokobanMapTest, testKSokoban0241) { testSingleMap("ksokoban0241"); }
TEST_F(SokobanMapTest, testKSokoban0242) { testSingleMap("ksokoban0242"); }
TEST_F(SokobanMapTest, testKSokoban0243) { testSingleMap("ksokoban0243"); }
TEST_F(SokobanMapTest, testKSokoban0244) { testSingleMap("ksokoban0244"); }
TEST_F(SokobanMapTest, testKSokoban0245) { testSingleMap("ksokoban0245"); }
TEST_F(SokobanMapTest, testKSokoban0246) { testSingleMap("ksokoban0246"); }
TEST_F(SokobanMapTest, testKSokoban0247) { testSingleMap("ksokoban0247"); }
TEST_F(SokobanMapTest, testKSokoban0248) { testSingleMap("ksokoban0248"); }
TEST_F(SokobanMapTest, testKSokoban0249) { testSingleMap("ksokoban0249"); }
TEST_F(SokobanMapTest, testKSokoban0250) { testSingleMap("ksokoban0250"); }
TEST_F(SokobanMapTest, testKSokoban0251) { testSingleMap("ksokoban0251"); }
TEST_F(SokobanMapTest, testKSokoban0252) { testSingleMap("ksokoban0252"); }
TEST_F(SokobanMapTest, testKSokoban0253) { testSingleMap("ksokoban0253"); }
TEST_F(SokobanMapTest, testKSokoban0254) { testSingleMap("ksokoban0254"); }
TEST_F(SokobanMapTest, testKSokoban0255) { testSingleMap("ksokoban0255"); }
TEST_F(SokobanMapTest, testKSokoban0256) { testSingleMap("ksokoban0256"); }
TEST_F(SokobanMapTest, testKSokoban0257) { testSingleMap("ksokoban0257"); }
TEST_F(SokobanMapTest, testKSokoban0258) { testSingleMap("ksokoban0258"); }
TEST_F(SokobanMapTest, testKSokoban0259) { testSingleMap("ksokoban0259"); }
TEST_F(SokobanMapTest, testKSokoban0260) { testSingleMap("ksokoban0260"); }
TEST_F(SokobanMapTest, testKSokoban0261) { testSingleMap("ksokoban0261"); }
TEST_F(SokobanMapTest, testKSokoban0262) { testSingleMap("ksokoban0262"); }
TEST_F(SokobanMapTest, testKSokoban0263) { testSingleMap("ksokoban0263"); }
TEST_F(SokobanMapTest, testKSokoban0264) { testSingleMap("ksokoban0264"); }
TEST_F(SokobanMapTest, testKSokoban0265) { testSingleMap("ksokoban0265"); }
TEST_F(SokobanMapTest, testKSokoban0266) { testSingleMap("ksokoban0266"); }
TEST_F(SokobanMapTest, testKSokoban0267) { testSingleMap("ksokoban0267"); }
TEST_F(SokobanMapTest, testKSokoban0268) { testSingleMap("ksokoban0268"); }
TEST_F(SokobanMapTest, testKSokoban0269) { testSingleMap("ksokoban0269"); }
TEST_F(SokobanMapTest, testKSokoban0270) { testSingleMap("ksokoban0270"); }
TEST_F(SokobanMapTest, testKSokoban0271) { testSingleMap("ksokoban0271"); }
TEST_F(SokobanMapTest, testKSokoban0272) { testSingleMap("ksokoban0272"); }
TEST_F(SokobanMapTest, testKSokoban0273) { testSingleMap("ksokoban0273"); }
TEST_F(SokobanMapTest, testKSokoban0274) { testSingleMap("ksokoban0274"); }
TEST_F(SokobanMapTest, testKSokoban0275) { testSingleMap("ksokoban0275"); }
TEST_F(SokobanMapTest, testKSokoban0276) { testSingleMap("ksokoban0276"); }
TEST_F(SokobanMapTest, testKSokoban0277) { testSingleMap("ksokoban0277"); }
TEST_F(SokobanMapTest, testKSokoban0278) { testSingleMap("ksokoban0278"); }
TEST_F(SokobanMapTest, testKSokoban0279) { testSingleMap("ksokoban0279"); }
TEST_F(SokobanMapTest, testKSokoban0280) { testSingleMap("ksokoban0280"); }
TEST_F(SokobanMapTest, testKSokoban0281) { testSingleMap("ksokoban0281"); }
TEST_F(SokobanMapTest, testKSokoban0282) { testSingleMap("ksokoban0282"); }
TEST_F(SokobanMapTest, testKSokoban0283) { testSingleMap("ksokoban0283"); }
TEST_F(SokobanMapTest, testKSokoban0284) { testSingleMap("ksokoban0284"); }
TEST_F(SokobanMapTest, testKSokoban0285) { testSingleMap("ksokoban0285"); }
TEST_F(SokobanMapTest, testKSokoban0286) { testSingleMap("ksokoban0286"); }
TEST_F(SokobanMapTest, testKSokoban0287) { testSingleMap("ksokoban0287"); }
TEST_F(SokobanMapTest, testKSokoban0288) { testSingleMap("ksokoban0288"); }
TEST_F(SokobanMapTest, testKSokoban0289) { testSingleMap("ksokoban0289"); }
TEST_F(SokobanMapTest, testKSokoban0290) { testSingleMap("ksokoban0290"); }
TEST_F(SokobanMapTest, testKSokoban0291) { testSingleMap("ksokoban0291"); }
TEST_F(SokobanMapTest, testKSokoban0292) { testSingleMap("ksokoban0292"); }
TEST_F(SokobanMapTest, testKSokoban0293) { testSingleMap("ksokoban0293"); }
TEST_F(SokobanMapTest, testKSokoban0294) { testSingleMap("ksokoban0294"); }
TEST_F(SokobanMapTest, testKSokoban0295) { testSingleMap("ksokoban0295"); }
TEST_F(SokobanMapTest, testKSokoban0296) { testSingleMap("ksokoban0296"); }
TEST_F(SokobanMapTest, testKSokoban0297) { testSingleMap("ksokoban0297"); }
TEST_F(SokobanMapTest, testKSokoban0298) { testSingleMap("ksokoban0298"); }
TEST_F(SokobanMapTest, testKSokoban0299) { testSingleMap("ksokoban0299"); }
TEST_F(SokobanMapTest, testKSokoban0300) { testSingleMap("ksokoban0300"); }
TEST_F(SokobanMapTest, testKSokoban0301) { testSingleMap("ksokoban0301"); }
TEST_F(SokobanMapTest, testKSokoban0302) { testSingleMap("ksokoban0302"); }
TEST_F(SokobanMapTest, testKSokoban0303) { testSingleMap("ksokoban0303"); }
TEST_F(SokobanMapTest, testKSokoban0304) { testSingleMap("ksokoban0304"); }
TEST_F(SokobanMapTest, testKSokoban0305) { testSingleMap("ksokoban0305"); }
TEST_F(SokobanMapTest, testKSokoban0306) { testSingleMap("ksokoban0306"); }
TEST_F(SokobanMapTest, testKSokoban0307) { testSingleMap("ksokoban0307"); }
TEST_F(SokobanMapTest, testKSokoban0308) { testSingleMap("ksokoban0308"); }
TEST_F(SokobanMapTest, testKSokoban0309) { testSingleMap("ksokoban0309"); }
TEST_F(SokobanMapTest, testKSokoban0310) { testSingleMap("ksokoban0310"); }
TEST_F(SokobanMapTest, testKSokoban0311) { testSingleMap("ksokoban0311"); }
TEST_F(SokobanMapTest, testKSokoban0312) { testSingleMap("ksokoban0312"); }
TEST_F(SokobanMapTest, testKSokoban0313) { testSingleMap("ksokoban0313"); }
TEST_F(SokobanMapTest, testKSokoban0314) { testSingleMap("ksokoban0314"); }
TEST_F(SokobanMapTest, testKSokoban0315) { testSingleMap("ksokoban0315"); }
TEST_F(SokobanMapTest, testKSokoban0316) { testSingleMap("ksokoban0316"); }
TEST_F(SokobanMapTest, testKSokoban0317) { testSingleMap("ksokoban0317"); }
TEST_F(SokobanMapTest, testKSokoban0318) { testSingleMap("ksokoban0318"); }
TEST_F(SokobanMapTest, testKSokoban0319) { testSingleMap("ksokoban0319"); }
TEST_F(SokobanMapTest, testKSokoban0320) { testSingleMap("ksokoban0320"); }
TEST_F(SokobanMapTest, testKSokoban0321) { testSingleMap("ksokoban0321"); }
TEST_F(SokobanMapTest, testKSokoban0322) { testSingleMap("ksokoban0322"); }
TEST_F(SokobanMapTest, testKSokoban0323) { testSingleMap("ksokoban0323"); }
TEST_F(SokobanMapTest, testKSokoban0324) { testSingleMap("ksokoban0324"); }
TEST_F(SokobanMapTest, testKSokoban0325) { testSingleMap("ksokoban0325"); }
TEST_F(SokobanMapTest, testKSokoban0326) { testSingleMap("ksokoban0326"); }
TEST_F(SokobanMapTest, testKSokoban0327) { testSingleMap("ksokoban0327"); }
TEST_F(SokobanMapTest, testKSokoban0328) { testSingleMap("ksokoban0328"); }
TEST_F(SokobanMapTest, testKSokoban0329) { testSingleMap("ksokoban0329"); }
TEST_F(SokobanMapTest, testKSokoban0330) { testSingleMap("ksokoban0330"); }
TEST_F(SokobanMapTest, testKSokoban0331) { testSingleMap("ksokoban0331"); }
TEST_F(SokobanMapTest, testKSokoban0332) { testSingleMap("ksokoban0332"); }
TEST_F(SokobanMapTest, testKSokoban0333) { testSingleMap("ksokoban0333"); }
TEST_F(SokobanMapTest, testKSokoban0334) { testSingleMap("ksokoban0334"); }
TEST_F(SokobanMapTest, testKSokoban0335) { testSingleMap("ksokoban0335"); }
TEST_F(SokobanMapTest, testKSokoban0336) { testSingleMap("ksokoban0336"); }
TEST_F(SokobanMapTest, testKSokoban0337) { testSingleMap("ksokoban0337"); }
TEST_F(SokobanMapTest, testKSokoban0338) { testSingleMap("ksokoban0338"); }
TEST_F(SokobanMapTest, testKSokoban0339) { testSingleMap("ksokoban0339"); }
TEST_F(SokobanMapTest, testKSokoban0340) { testSingleMap("ksokoban0340"); }
TEST_F(SokobanMapTest, testKSokoban0341) { testSingleMap("ksokoban0341"); }
TEST_F(SokobanMapTest, testKSokoban0342) { testSingleMap("ksokoban0342"); }
TEST_F(SokobanMapTest, testKSokoban0343) { testSingleMap("ksokoban0343"); }
TEST_F(SokobanMapTest, testKSokoban0344) { testSingleMap("ksokoban0344"); }
TEST_F(SokobanMapTest, testKSokoban0345) { testSingleMap("ksokoban0345"); }
TEST_F(SokobanMapTest, testKSokoban0346) { testSingleMap("ksokoban0346"); }
TEST_F(SokobanMapTest, testKSokoban0347) { testSingleMap("ksokoban0347"); }
TEST_F(SokobanMapTest, testKSokoban0348) { testSingleMap("ksokoban0348"); }
TEST_F(SokobanMapTest, testKSokoban0349) { testSingleMap("ksokoban0349"); }
TEST_F(SokobanMapTest, testKSokoban0350) { testSingleMap("ksokoban0350"); }
TEST_F(SokobanMapTest, testKSokoban0351) { testSingleMap("ksokoban0351"); }
TEST_F(SokobanMapTest, testKSokoban0352) { testSingleMap("ksokoban0352"); }
TEST_F(SokobanMapTest, testKSokoban0353) { testSingleMap("ksokoban0353"); }
TEST_F(SokobanMapTest, testKSokoban0354) { testSingleMap("ksokoban0354"); }
TEST_F(SokobanMapTest, testgri0001) { testSingleMap("gri0001"); }
TEST_F(SokobanMapTest, testgri0002) { testSingleMap("gri0002"); }
TEST_F(SokobanMapTest, testgri0003) { testSingleMap("gri0003"); }
TEST_F(SokobanMapTest, testgri0004) { testSingleMap("gri0004"); }
TEST_F(SokobanMapTest, testgri0005) { testSingleMap("gri0005"); }
TEST_F(SokobanMapTest, testgri0006) { testSingleMap("gri0006"); }
TEST_F(SokobanMapTest, testgri0007) { testSingleMap("gri0007"); }
TEST_F(SokobanMapTest, testgri0008) { testSingleMap("gri0008"); }
TEST_F(SokobanMapTest, testgri0009) { testSingleMap("gri0009"); }
TEST_F(SokobanMapTest, testgri0010) { testSingleMap("gri0010"); }
TEST_F(SokobanMapTest, testgri0011) { testSingleMap("gri0011"); }
TEST_F(SokobanMapTest, testgri0012) { testSingleMap("gri0012"); }
TEST_F(SokobanMapTest, testgri0013) { testSingleMap("gri0013"); }
TEST_F(SokobanMapTest, testgri0014) { testSingleMap("gri0014"); }
TEST_F(SokobanMapTest, testgri0015) { testSingleMap("gri0015"); }
TEST_F(SokobanMapTest, testgri0016) { testSingleMap("gri0016"); }
TEST_F(SokobanMapTest, testgri0017) { testSingleMap("gri0017"); }
TEST_F(SokobanMapTest, testgri0018) { testSingleMap("gri0018"); }
TEST_F(SokobanMapTest, testgri0019) { testSingleMap("gri0019"); }
TEST_F(SokobanMapTest, testgri0020) { testSingleMap("gri0020"); }
TEST_F(SokobanMapTest, testgri0021) { testSingleMap("gri0021"); }
TEST_F(SokobanMapTest, testgri0022) { testSingleMap("gri0022"); }
TEST_F(SokobanMapTest, testgri0023) { testSingleMap("gri0023"); }
TEST_F(SokobanMapTest, testgri0024) { testSingleMap("gri0024"); }
TEST_F(SokobanMapTest, testgri0025) { testSingleMap("gri0025"); }
TEST_F(SokobanMapTest, testgri0026) { testSingleMap("gri0026"); }
TEST_F(SokobanMapTest, testgri0027) { testSingleMap("gri0027"); }
TEST_F(SokobanMapTest, testgri0028) { testSingleMap("gri0028"); }
TEST_F(SokobanMapTest, testgri0029) { testSingleMap("gri0029"); }
TEST_F(SokobanMapTest, testgri0030) { testSingleMap("gri0030"); }
TEST_F(SokobanMapTest, testgri0031) { testSingleMap("gri0031"); }
TEST_F(SokobanMapTest, testgri0032) { testSingleMap("gri0032"); }
TEST_F(SokobanMapTest, testgri0033) { testSingleMap("gri0033"); }
TEST_F(SokobanMapTest, testgri0034) { testSingleMap("gri0034"); }
TEST_F(SokobanMapTest, testgri0035) { testSingleMap("gri0035"); }
TEST_F(SokobanMapTest, testgri0036) { testSingleMap("gri0036"); }
TEST_F(SokobanMapTest, testgri0037) { testSingleMap("gri0037"); }
TEST_F(SokobanMapTest, testgri0038) { testSingleMap("gri0038"); }
TEST_F(SokobanMapTest, testgri0039) { testSingleMap("gri0039"); }
TEST_F(SokobanMapTest, testgri0040) { testSingleMap("gri0040"); }
TEST_F(SokobanMapTest, testgri0041) { testSingleMap("gri0041"); }
TEST_F(SokobanMapTest, testgri0042) { testSingleMap("gri0042"); }
TEST_F(SokobanMapTest, testgri0043) { testSingleMap("gri0043"); }
TEST_F(SokobanMapTest, testgri0044) { testSingleMap("gri0044"); }
TEST_F(SokobanMapTest, testgri0045) { testSingleMap("gri0045"); }
TEST_F(SokobanMapTest, testgri0046) { testSingleMap("gri0046"); }
TEST_F(SokobanMapTest, testgri0047) { testSingleMap("gri0047"); }
TEST_F(SokobanMapTest, testgri0048) { testSingleMap("gri0048"); }
TEST_F(SokobanMapTest, testgri0049) { testSingleMap("gri0049"); }
TEST_F(SokobanMapTest, testgri0050) { testSingleMap("gri0050"); }
TEST_F(SokobanMapTest, testgri0051) { testSingleMap("gri0051"); }
TEST_F(SokobanMapTest, testgri0052) { testSingleMap("gri0052"); }
TEST_F(SokobanMapTest, testgri0053) { testSingleMap("gri0053"); }
TEST_F(SokobanMapTest, testgri0054) { testSingleMap("gri0054"); }
TEST_F(SokobanMapTest, testgri0055) { testSingleMap("gri0055"); }
TEST_F(SokobanMapTest, testgri0056) { testSingleMap("gri0056"); }
TEST_F(SokobanMapTest, testgri0057) { testSingleMap("gri0057"); }
TEST_F(SokobanMapTest, testgri0058) { testSingleMap("gri0058"); }
TEST_F(SokobanMapTest, testgri0059) { testSingleMap("gri0059"); }
TEST_F(SokobanMapTest, testgri0060) { testSingleMap("gri0060"); }
TEST_F(SokobanMapTest, testgri0061) { testSingleMap("gri0061"); }
TEST_F(SokobanMapTest, testgri0062) { testSingleMap("gri0062"); }
TEST_F(SokobanMapTest, testgri0063) { testSingleMap("gri0063"); }
TEST_F(SokobanMapTest, testgri0064) { testSingleMap("gri0064"); }
TEST_F(SokobanMapTest, testgri0065) { testSingleMap("gri0065"); }
TEST_F(SokobanMapTest, testgri0066) { testSingleMap("gri0066"); }
TEST_F(SokobanMapTest, testgri0067) { testSingleMap("gri0067"); }
TEST_F(SokobanMapTest, testgri0068) { testSingleMap("gri0068"); }
TEST_F(SokobanMapTest, testgri0069) { testSingleMap("gri0069"); }
TEST_F(SokobanMapTest, testgri0070) { testSingleMap("gri0070"); }
TEST_F(SokobanMapTest, testgri0071) { testSingleMap("gri0071"); }
TEST_F(SokobanMapTest, testgri0072) { testSingleMap("gri0072"); }
TEST_F(SokobanMapTest, testgri0073) { testSingleMap("gri0073"); }
TEST_F(SokobanMapTest, testgri0074) { testSingleMap("gri0074"); }
TEST_F(SokobanMapTest, testgri0075) { testSingleMap("gri0075"); }
TEST_F(SokobanMapTest, testgri0076) { testSingleMap("gri0076"); }
TEST_F(SokobanMapTest, testgri0077) { testSingleMap("gri0077"); }
TEST_F(SokobanMapTest, testgri0078) { testSingleMap("gri0078"); }
TEST_F(SokobanMapTest, testgri0079) { testSingleMap("gri0079"); }
TEST_F(SokobanMapTest, testgri0080) { testSingleMap("gri0080"); }
TEST_F(SokobanMapTest, testgri0081) { testSingleMap("gri0081"); }
TEST_F(SokobanMapTest, testgri0082) { testSingleMap("gri0082"); }
TEST_F(SokobanMapTest, testgri0083) { testSingleMap("gri0083"); }
TEST_F(SokobanMapTest, testgri0084) { testSingleMap("gri0084"); }
TEST_F(SokobanMapTest, testgri0085) { testSingleMap("gri0085"); }
TEST_F(SokobanMapTest, testgri0086) { testSingleMap("gri0086"); }
TEST_F(SokobanMapTest, testgri0087) { testSingleMap("gri0087"); }
TEST_F(SokobanMapTest, testgri0088) { testSingleMap("gri0088"); }
TEST_F(SokobanMapTest, testgri0089) { testSingleMap("gri0089"); }
TEST_F(SokobanMapTest, testgri0090) { testSingleMap("gri0090"); }
TEST_F(SokobanMapTest, testgri0091) { testSingleMap("gri0091"); }
TEST_F(SokobanMapTest, testgri0092) { testSingleMap("gri0092"); }
TEST_F(SokobanMapTest, testgri0093) { testSingleMap("gri0093"); }
TEST_F(SokobanMapTest, testgri0094) { testSingleMap("gri0094"); }
TEST_F(SokobanMapTest, testgri0095) { testSingleMap("gri0095"); }
TEST_F(SokobanMapTest, testgri0096) { testSingleMap("gri0096"); }
TEST_F(SokobanMapTest, testgri0097) { testSingleMap("gri0097"); }
TEST_F(SokobanMapTest, testgri0098) { testSingleMap("gri0098"); }
TEST_F(SokobanMapTest, testgri0099) { testSingleMap("gri0099"); }
TEST_F(SokobanMapTest, testgri0100) { testSingleMap("gri0100"); }
TEST_F(SokobanMapTest, testgri0101) { testSingleMap("gri0101"); }
TEST_F(SokobanMapTest, testgri0102) { testSingleMap("gri0102"); }
TEST_F(SokobanMapTest, testgri0103) { testSingleMap("gri0103"); }
TEST_F(SokobanMapTest, testgri0104) { testSingleMap("gri0104"); }
TEST_F(SokobanMapTest, testgri0105) { testSingleMap("gri0105"); }
TEST_F(SokobanMapTest, testgri0106) { testSingleMap("gri0106"); }
TEST_F(SokobanMapTest, testgri0107) { testSingleMap("gri0107"); }
TEST_F(SokobanMapTest, testgri0108) { testSingleMap("gri0108"); }
TEST_F(SokobanMapTest, testgri0109) { testSingleMap("gri0109"); }
TEST_F(SokobanMapTest, testgri0110) { testSingleMap("gri0110"); }
TEST_F(SokobanMapTest, testgri0111) { testSingleMap("gri0111"); }
TEST_F(SokobanMapTest, testgri0112) { testSingleMap("gri0112"); }
TEST_F(SokobanMapTest, testgri0113) { testSingleMap("gri0113"); }
TEST_F(SokobanMapTest, testgri0114) { testSingleMap("gri0114"); }
TEST_F(SokobanMapTest, testgri0115) { testSingleMap("gri0115"); }
TEST_F(SokobanMapTest, testgri0116) { testSingleMap("gri0116"); }
TEST_F(SokobanMapTest, testgri0117) { testSingleMap("gri0117"); }
TEST_F(SokobanMapTest, testgri0118) { testSingleMap("gri0118"); }
TEST_F(SokobanMapTest, testgri0119) { testSingleMap("gri0119"); }
TEST_F(SokobanMapTest, testgri0120) { testSingleMap("gri0120"); }
TEST_F(SokobanMapTest, testgri0121) { testSingleMap("gri0121"); }
TEST_F(SokobanMapTest, testgri0122) { testSingleMap("gri0122"); }
TEST_F(SokobanMapTest, testgri0123) { testSingleMap("gri0123"); }
TEST_F(SokobanMapTest, testgri0124) { testSingleMap("gri0124"); }
TEST_F(SokobanMapTest, testgri0125) { testSingleMap("gri0125"); }
TEST_F(SokobanMapTest, testgri0126) { testSingleMap("gri0126"); }
TEST_F(SokobanMapTest, testgri0127) { testSingleMap("gri0127"); }
TEST_F(SokobanMapTest, testgri0128) { testSingleMap("gri0128"); }
TEST_F(SokobanMapTest, testgri0129) { testSingleMap("gri0129"); }
TEST_F(SokobanMapTest, testgri0130) { testSingleMap("gri0130"); }
TEST_F(SokobanMapTest, testgri0131) { testSingleMap("gri0131"); }
TEST_F(SokobanMapTest, testgri0132) { testSingleMap("gri0132"); }
TEST_F(SokobanMapTest, testgri0133) { testSingleMap("gri0133"); }
TEST_F(SokobanMapTest, testgri0134) { testSingleMap("gri0134"); }
TEST_F(SokobanMapTest, testgri0135) { testSingleMap("gri0135"); }
TEST_F(SokobanMapTest, testgri0136) { testSingleMap("gri0136"); }
TEST_F(SokobanMapTest, testgri0137) { testSingleMap("gri0137"); }
TEST_F(SokobanMapTest, testgri0138) { testSingleMap("gri0138"); }
TEST_F(SokobanMapTest, testgri0139) { testSingleMap("gri0139"); }
TEST_F(SokobanMapTest, testgri0140) { testSingleMap("gri0140"); }
TEST_F(SokobanMapTest, testgrigrcomet0001) { testSingleMap("grigrcomet0001"); }
TEST_F(SokobanMapTest, testgrigrcomet0002) { testSingleMap("grigrcomet0002"); }
TEST_F(SokobanMapTest, testgrigrcomet0003) { testSingleMap("grigrcomet0003"); }
TEST_F(SokobanMapTest, testgrigrcomet0004) { testSingleMap("grigrcomet0004"); }
TEST_F(SokobanMapTest, testgrigrcomet0005) { testSingleMap("grigrcomet0005"); }
TEST_F(SokobanMapTest, testgrigrcomet0006) { testSingleMap("grigrcomet0006"); }
TEST_F(SokobanMapTest, testgrigrcomet0007) { testSingleMap("grigrcomet0007"); }
TEST_F(SokobanMapTest, testgrigrcomet0008) { testSingleMap("grigrcomet0008"); }
TEST_F(SokobanMapTest, testgrigrcomet0009) { testSingleMap("grigrcomet0009"); }
TEST_F(SokobanMapTest, testgrigrcomet0010) { testSingleMap("grigrcomet0010"); }
TEST_F(SokobanMapTest, testgrigrcomet0011) { testSingleMap("grigrcomet0011"); }
TEST_F(SokobanMapTest, testgrigrcomet0012) { testSingleMap("grigrcomet0012"); }
TEST_F(SokobanMapTest, testgrigrcomet0013) { testSingleMap("grigrcomet0013"); }
TEST_F(SokobanMapTest, testgrigrcomet0014) { testSingleMap("grigrcomet0014"); }
TEST_F(SokobanMapTest, testgrigrcomet0015) { testSingleMap("grigrcomet0015"); }
TEST_F(SokobanMapTest, testgrigrcomet0016) { testSingleMap("grigrcomet0016"); }
TEST_F(SokobanMapTest, testgrigrcomet0017) { testSingleMap("grigrcomet0017"); }
TEST_F(SokobanMapTest, testgrigrcomet0018) { testSingleMap("grigrcomet0018"); }
TEST_F(SokobanMapTest, testgrigrcomet0019) { testSingleMap("grigrcomet0019"); }
TEST_F(SokobanMapTest, testgrigrcomet0020) { testSingleMap("grigrcomet0020"); }
TEST_F(SokobanMapTest, testgrigrcomet0021) { testSingleMap("grigrcomet0021"); }
TEST_F(SokobanMapTest, testgrigrcomet0022) { testSingleMap("grigrcomet0022"); }
TEST_F(SokobanMapTest, testgrigrcomet0023) { testSingleMap("grigrcomet0023"); }
TEST_F(SokobanMapTest, testgrigrcomet0024) { testSingleMap("grigrcomet0024"); }
TEST_F(SokobanMapTest, testgrigrcomet0025) { testSingleMap("grigrcomet0025"); }
TEST_F(SokobanMapTest, testgrigrcomet0026) { testSingleMap("grigrcomet0026"); }
TEST_F(SokobanMapTest, testgrigrcomet0027) { testSingleMap("grigrcomet0027"); }
TEST_F(SokobanMapTest, testgrigrcomet0028) { testSingleMap("grigrcomet0028"); }
TEST_F(SokobanMapTest, testgrigrcomet0029) { testSingleMap("grigrcomet0029"); }
TEST_F(SokobanMapTest, testgrigrcomet0030) { testSingleMap("grigrcomet0030"); }
TEST_F(SokobanMapTest, testgrigrspecial0001) { testSingleMap("grigrspecial0001"); }
TEST_F(SokobanMapTest, testgrigrspecial0002) { testSingleMap("grigrspecial0002"); }
TEST_F(SokobanMapTest, testgrigrspecial0003) { testSingleMap("grigrspecial0003"); }
TEST_F(SokobanMapTest, testgrigrspecial0004) { testSingleMap("grigrspecial0004"); }
TEST_F(SokobanMapTest, testgrigrspecial0005) { testSingleMap("grigrspecial0005"); }
TEST_F(SokobanMapTest, testgrigrspecial0006) { testSingleMap("grigrspecial0006"); }
TEST_F(SokobanMapTest, testgrigrspecial0007) { testSingleMap("grigrspecial0007"); }
TEST_F(SokobanMapTest, testgrigrspecial0008) { testSingleMap("grigrspecial0008"); }
TEST_F(SokobanMapTest, testgrigrspecial0009) { testSingleMap("grigrspecial0009"); }
TEST_F(SokobanMapTest, testgrigrspecial0010) { testSingleMap("grigrspecial0010"); }
TEST_F(SokobanMapTest, testgrigrspecial0011) { testSingleMap("grigrspecial0011"); }
TEST_F(SokobanMapTest, testgrigrspecial0012) { testSingleMap("grigrspecial0012"); }
TEST_F(SokobanMapTest, testgrigrspecial0013) { testSingleMap("grigrspecial0013"); }
TEST_F(SokobanMapTest, testgrigrspecial0014) { testSingleMap("grigrspecial0014"); }
TEST_F(SokobanMapTest, testgrigrspecial0015) { testSingleMap("grigrspecial0015"); }
TEST_F(SokobanMapTest, testgrigrspecial0016) { testSingleMap("grigrspecial0016"); }
TEST_F(SokobanMapTest, testgrigrspecial0017) { testSingleMap("grigrspecial0017"); }
TEST_F(SokobanMapTest, testgrigrspecial0018) { testSingleMap("grigrspecial0018"); }
TEST_F(SokobanMapTest, testgrigrspecial0019) { testSingleMap("grigrspecial0019"); }
TEST_F(SokobanMapTest, testgrigrspecial0020) { testSingleMap("grigrspecial0020"); }
TEST_F(SokobanMapTest, testgrigrspecial0021) { testSingleMap("grigrspecial0021"); }
TEST_F(SokobanMapTest, testgrigrspecial0022) { testSingleMap("grigrspecial0022"); }
TEST_F(SokobanMapTest, testgrigrspecial0023) { testSingleMap("grigrspecial0023"); }
TEST_F(SokobanMapTest, testgrigrspecial0024) { testSingleMap("grigrspecial0024"); }
TEST_F(SokobanMapTest, testgrigrspecial0025) { testSingleMap("grigrspecial0025"); }
TEST_F(SokobanMapTest, testgrigrspecial0026) { testSingleMap("grigrspecial0026"); }
TEST_F(SokobanMapTest, testgrigrspecial0027) { testSingleMap("grigrspecial0027"); }
TEST_F(SokobanMapTest, testgrigrspecial0028) { testSingleMap("grigrspecial0028"); }
TEST_F(SokobanMapTest, testgrigrspecial0029) { testSingleMap("grigrspecial0029"); }
TEST_F(SokobanMapTest, testgrigrspecial0030) { testSingleMap("grigrspecial0030"); }
TEST_F(SokobanMapTest, testgrigrspecial0031) { testSingleMap("grigrspecial0031"); }
TEST_F(SokobanMapTest, testgrigrspecial0032) { testSingleMap("grigrspecial0032"); }
TEST_F(SokobanMapTest, testgrigrspecial0033) { testSingleMap("grigrspecial0033"); }
TEST_F(SokobanMapTest, testgrigrspecial0034) { testSingleMap("grigrspecial0034"); }
TEST_F(SokobanMapTest, testgrigrspecial0035) { testSingleMap("grigrspecial0035"); }
TEST_F(SokobanMapTest, testgrigrspecial0036) { testSingleMap("grigrspecial0036"); }
TEST_F(SokobanMapTest, testgrigrspecial0037) { testSingleMap("grigrspecial0037"); }
TEST_F(SokobanMapTest, testgrigrspecial0038) { testSingleMap("grigrspecial0038"); }
TEST_F(SokobanMapTest, testgrigrspecial0039) { testSingleMap("grigrspecial0039"); }
TEST_F(SokobanMapTest, testgrigrspecial0040) { testSingleMap("grigrspecial0040"); }
TEST_F(SokobanMapTest, testgrigrstar0001) { testSingleMap("grigrstar0001"); }
TEST_F(SokobanMapTest, testgrigrstar0002) { testSingleMap("grigrstar0002"); }
TEST_F(SokobanMapTest, testgrigrstar0003) { testSingleMap("grigrstar0003"); }
TEST_F(SokobanMapTest, testgrigrstar0004) { testSingleMap("grigrstar0004"); }
TEST_F(SokobanMapTest, testgrigrstar0005) { testSingleMap("grigrstar0005"); }
TEST_F(SokobanMapTest, testgrigrstar0006) { testSingleMap("grigrstar0006"); }
TEST_F(SokobanMapTest, testgrigrstar0007) { testSingleMap("grigrstar0007"); }
TEST_F(SokobanMapTest, testgrigrstar0008) { testSingleMap("grigrstar0008"); }
TEST_F(SokobanMapTest, testgrigrstar0009) { testSingleMap("grigrstar0009"); }
TEST_F(SokobanMapTest, testgrigrstar0010) { testSingleMap("grigrstar0010"); }
TEST_F(SokobanMapTest, testgrigrstar0011) { testSingleMap("grigrstar0011"); }
TEST_F(SokobanMapTest, testgrigrstar0012) { testSingleMap("grigrstar0012"); }
TEST_F(SokobanMapTest, testgrigrstar0013) { testSingleMap("grigrstar0013"); }
TEST_F(SokobanMapTest, testgrigrstar0014) { testSingleMap("grigrstar0014"); }
TEST_F(SokobanMapTest, testgrigrstar0015) { testSingleMap("grigrstar0015"); }
TEST_F(SokobanMapTest, testgrigrstar0016) { testSingleMap("grigrstar0016"); }
TEST_F(SokobanMapTest, testgrigrstar0017) { testSingleMap("grigrstar0017"); }
TEST_F(SokobanMapTest, testgrigrstar0018) { testSingleMap("grigrstar0018"); }
TEST_F(SokobanMapTest, testgrigrstar0019) { testSingleMap("grigrstar0019"); }
TEST_F(SokobanMapTest, testgrigrstar0020) { testSingleMap("grigrstar0020"); }
TEST_F(SokobanMapTest, testgrigrstar0021) { testSingleMap("grigrstar0021"); }
TEST_F(SokobanMapTest, testgrigrstar0022) { testSingleMap("grigrstar0022"); }
TEST_F(SokobanMapTest, testgrigrstar0023) { testSingleMap("grigrstar0023"); }
TEST_F(SokobanMapTest, testgrigrstar0024) { testSingleMap("grigrstar0024"); }
TEST_F(SokobanMapTest, testgrigrstar0025) { testSingleMap("grigrstar0025"); }
TEST_F(SokobanMapTest, testgrigrstar0026) { testSingleMap("grigrstar0026"); }
TEST_F(SokobanMapTest, testgrigrstar0027) { testSingleMap("grigrstar0027"); }
TEST_F(SokobanMapTest, testgrigrstar0028) { testSingleMap("grigrstar0028"); }
TEST_F(SokobanMapTest, testgrigrstar0029) { testSingleMap("grigrstar0029"); }
TEST_F(SokobanMapTest, testgrigrstar0030) { testSingleMap("grigrstar0030"); }
TEST_F(SokobanMapTest, testgrigrsun0001) { testSingleMap("grigrsun0001"); }
TEST_F(SokobanMapTest, testgrigrsun0002) { testSingleMap("grigrsun0002"); }
TEST_F(SokobanMapTest, testgrigrsun0003) { testSingleMap("grigrsun0003"); }
TEST_F(SokobanMapTest, testgrigrsun0004) { testSingleMap("grigrsun0004"); }
TEST_F(SokobanMapTest, testgrigrsun0005) { testSingleMap("grigrsun0005"); }
TEST_F(SokobanMapTest, testgrigrsun0006) { testSingleMap("grigrsun0006"); }
TEST_F(SokobanMapTest, testgrigrsun0007) { testSingleMap("grigrsun0007"); }
TEST_F(SokobanMapTest, testgrigrsun0008) { testSingleMap("grigrsun0008"); }
TEST_F(SokobanMapTest, testgrigrsun0009) { testSingleMap("grigrsun0009"); }
TEST_F(SokobanMapTest, testgrigrsun0010) { testSingleMap("grigrsun0010"); }
