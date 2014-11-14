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
// TODO: segfault
TEST_F(SokobanMapTest, testgri0048) { testSingleMap("gri0048"); }
TEST_F(SokobanMapTest, testgri0049) { testSingleMap("gri0049"); }
TEST_F(SokobanMapTest, testgri0050) { testSingleMap("gri0050"); }
TEST_F(SokobanMapTest, testgri0051) { testSingleMap("gri0051"); }
TEST_F(SokobanMapTest, testgri0052) { testSingleMap("gri0052"); }
// TODO: segfault
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
TEST_F(SokobanMapTest, testmicroban01_0001) { testSingleMap("microban01_0001"); }
TEST_F(SokobanMapTest, testmicroban01_0002) { testSingleMap("microban01_0002"); }
TEST_F(SokobanMapTest, testmicroban01_0003) { testSingleMap("microban01_0003"); }
TEST_F(SokobanMapTest, testmicroban01_0004) { testSingleMap("microban01_0004"); }
TEST_F(SokobanMapTest, testmicroban01_0005) { testSingleMap("microban01_0005"); }
TEST_F(SokobanMapTest, testmicroban01_0006) { testSingleMap("microban01_0006"); }
TEST_F(SokobanMapTest, testmicroban01_0007) { testSingleMap("microban01_0007"); }
TEST_F(SokobanMapTest, testmicroban01_0008) { testSingleMap("microban01_0008"); }
TEST_F(SokobanMapTest, testmicroban01_0009) { testSingleMap("microban01_0009"); }
TEST_F(SokobanMapTest, testmicroban01_0010) { testSingleMap("microban01_0010"); }
// TODO: segfault
TEST_F(SokobanMapTest, testmicroban01_0011) { testSingleMap("microban01_0011"); }
TEST_F(SokobanMapTest, testmicroban01_0012) { testSingleMap("microban01_0012"); }
TEST_F(SokobanMapTest, testmicroban01_0013) { testSingleMap("microban01_0013"); }
TEST_F(SokobanMapTest, testmicroban01_0014) { testSingleMap("microban01_0014"); }
TEST_F(SokobanMapTest, testmicroban01_0015) { testSingleMap("microban01_0015"); }
TEST_F(SokobanMapTest, testmicroban01_0016) { testSingleMap("microban01_0016"); }
TEST_F(SokobanMapTest, testmicroban01_0017) { testSingleMap("microban01_0017"); }
TEST_F(SokobanMapTest, testmicroban01_0018) { testSingleMap("microban01_0018"); }
TEST_F(SokobanMapTest, testmicroban01_0019) { testSingleMap("microban01_0019"); }
TEST_F(SokobanMapTest, testmicroban01_0020) { testSingleMap("microban01_0020"); }
TEST_F(SokobanMapTest, testmicroban01_0021) { testSingleMap("microban01_0021"); }
TEST_F(SokobanMapTest, testmicroban01_0022) { testSingleMap("microban01_0022"); }
TEST_F(SokobanMapTest, testmicroban01_0023) { testSingleMap("microban01_0023"); }
TEST_F(SokobanMapTest, testmicroban01_0024) { testSingleMap("microban01_0024"); }
TEST_F(SokobanMapTest, testmicroban01_0025) { testSingleMap("microban01_0025"); }
TEST_F(SokobanMapTest, testmicroban01_0026) { testSingleMap("microban01_0026"); }
TEST_F(SokobanMapTest, testmicroban01_0027) { testSingleMap("microban01_0027"); }
TEST_F(SokobanMapTest, testmicroban01_0028) { testSingleMap("microban01_0028"); }
TEST_F(SokobanMapTest, testmicroban01_0029) { testSingleMap("microban01_0029"); }
TEST_F(SokobanMapTest, testmicroban01_0030) { testSingleMap("microban01_0030"); }
TEST_F(SokobanMapTest, testmicroban01_0031) { testSingleMap("microban01_0031"); }
TEST_F(SokobanMapTest, testmicroban01_0032) { testSingleMap("microban01_0032"); }
TEST_F(SokobanMapTest, testmicroban01_0033) { testSingleMap("microban01_0033"); }
TEST_F(SokobanMapTest, testmicroban01_0034) { testSingleMap("microban01_0034"); }
TEST_F(SokobanMapTest, testmicroban01_0035) { testSingleMap("microban01_0035"); }
TEST_F(SokobanMapTest, testmicroban01_0036) { testSingleMap("microban01_0036"); }
// TODO: segfault
TEST_F(SokobanMapTest, testmicroban01_0037) { testSingleMap("microban01_0037"); }
TEST_F(SokobanMapTest, testmicroban01_0038) { testSingleMap("microban01_0038"); }
TEST_F(SokobanMapTest, testmicroban01_0039) { testSingleMap("microban01_0039"); }
TEST_F(SokobanMapTest, testmicroban01_0040) { testSingleMap("microban01_0040"); }
TEST_F(SokobanMapTest, testmicroban01_0041) { testSingleMap("microban01_0041"); }
TEST_F(SokobanMapTest, testmicroban01_0042) { testSingleMap("microban01_0042"); }
// TODO: segfault
TEST_F(SokobanMapTest, testmicroban01_0043) { testSingleMap("microban01_0043"); }
TEST_F(SokobanMapTest, testmicroban01_0044) { testSingleMap("microban01_0044"); }
TEST_F(SokobanMapTest, testmicroban01_0045) { testSingleMap("microban01_0045"); }
TEST_F(SokobanMapTest, testmicroban01_0046) { testSingleMap("microban01_0046"); }
TEST_F(SokobanMapTest, testmicroban01_0047) { testSingleMap("microban01_0047"); }
// TODO: segfault
TEST_F(SokobanMapTest, testmicroban01_0048) { testSingleMap("microban01_0048"); }
TEST_F(SokobanMapTest, testmicroban01_0049) { testSingleMap("microban01_0049"); }
TEST_F(SokobanMapTest, testmicroban01_0050) { testSingleMap("microban01_0050"); }
TEST_F(SokobanMapTest, testmicroban01_0051) { testSingleMap("microban01_0051"); }
TEST_F(SokobanMapTest, testmicroban01_0052) { testSingleMap("microban01_0052"); }
TEST_F(SokobanMapTest, testmicroban01_0053) { testSingleMap("microban01_0053"); }
TEST_F(SokobanMapTest, testmicroban01_0054) { testSingleMap("microban01_0054"); }
// TODO: segfault
TEST_F(SokobanMapTest, testmicroban01_0055) { testSingleMap("microban01_0055"); }
TEST_F(SokobanMapTest, testmicroban01_0056) { testSingleMap("microban01_0056"); }
TEST_F(SokobanMapTest, testmicroban01_0057) { testSingleMap("microban01_0057"); }
TEST_F(SokobanMapTest, testmicroban01_0058) { testSingleMap("microban01_0058"); }
TEST_F(SokobanMapTest, testmicroban01_0059) { testSingleMap("microban01_0059"); }
TEST_F(SokobanMapTest, testmicroban01_0060) { testSingleMap("microban01_0060"); }
TEST_F(SokobanMapTest, testmicroban01_0061) { testSingleMap("microban01_0061"); }
TEST_F(SokobanMapTest, testmicroban01_0062) { testSingleMap("microban01_0062"); }
TEST_F(SokobanMapTest, testmicroban01_0063) { testSingleMap("microban01_0063"); }
TEST_F(SokobanMapTest, testmicroban01_0064) { testSingleMap("microban01_0064"); }
TEST_F(SokobanMapTest, testmicroban01_0065) { testSingleMap("microban01_0065"); }
TEST_F(SokobanMapTest, testmicroban01_0066) { testSingleMap("microban01_0066"); }
TEST_F(SokobanMapTest, testmicroban01_0067) { testSingleMap("microban01_0067"); }
TEST_F(SokobanMapTest, testmicroban01_0068) { testSingleMap("microban01_0068"); }
TEST_F(SokobanMapTest, testmicroban01_0069) { testSingleMap("microban01_0069"); }
TEST_F(SokobanMapTest, testmicroban01_0070) { testSingleMap("microban01_0070"); }
TEST_F(SokobanMapTest, testmicroban01_0071) { testSingleMap("microban01_0071"); }
TEST_F(SokobanMapTest, testmicroban01_0072) { testSingleMap("microban01_0072"); }
TEST_F(SokobanMapTest, testmicroban01_0073) { testSingleMap("microban01_0073"); }
TEST_F(SokobanMapTest, testmicroban01_0074) { testSingleMap("microban01_0074"); }
TEST_F(SokobanMapTest, testmicroban01_0075) { testSingleMap("microban01_0075"); }
TEST_F(SokobanMapTest, testmicroban01_0076) { testSingleMap("microban01_0076"); }
TEST_F(SokobanMapTest, testmicroban01_0077) { testSingleMap("microban01_0077"); }
TEST_F(SokobanMapTest, testmicroban01_0078) { testSingleMap("microban01_0078"); }
// TODO: segfault
TEST_F(SokobanMapTest, testmicroban01_0079) { testSingleMap("microban01_0079"); }
TEST_F(SokobanMapTest, testmicroban01_0080) { testSingleMap("microban01_0080"); }
TEST_F(SokobanMapTest, testmicroban01_0081) { testSingleMap("microban01_0081"); }
// TODO: segfault
TEST_F(SokobanMapTest, testmicroban01_0082) { testSingleMap("microban01_0082"); }
TEST_F(SokobanMapTest, testmicroban01_0083) { testSingleMap("microban01_0083"); }
TEST_F(SokobanMapTest, testmicroban01_0084) { testSingleMap("microban01_0084"); }
TEST_F(SokobanMapTest, testmicroban01_0085) { testSingleMap("microban01_0085"); }
TEST_F(SokobanMapTest, testmicroban01_0086) { testSingleMap("microban01_0086"); }
TEST_F(SokobanMapTest, testmicroban01_0087) { testSingleMap("microban01_0087"); }
TEST_F(SokobanMapTest, testmicroban01_0088) { testSingleMap("microban01_0088"); }
TEST_F(SokobanMapTest, testmicroban01_0089) { testSingleMap("microban01_0089"); }
TEST_F(SokobanMapTest, testmicroban01_0090) { testSingleMap("microban01_0090"); }
TEST_F(SokobanMapTest, testmicroban01_0091) { testSingleMap("microban01_0091"); }
TEST_F(SokobanMapTest, testmicroban01_0092) { testSingleMap("microban01_0092"); }
TEST_F(SokobanMapTest, testmicroban01_0093) { testSingleMap("microban01_0093"); }
TEST_F(SokobanMapTest, testmicroban01_0094) { testSingleMap("microban01_0094"); }
TEST_F(SokobanMapTest, testmicroban01_0095) { testSingleMap("microban01_0095"); }
TEST_F(SokobanMapTest, testmicroban01_0096) { testSingleMap("microban01_0096"); }
TEST_F(SokobanMapTest, testmicroban01_0097) { testSingleMap("microban01_0097"); }
TEST_F(SokobanMapTest, testmicroban01_0098) { testSingleMap("microban01_0098"); }
TEST_F(SokobanMapTest, testmicroban01_0099) { testSingleMap("microban01_0099"); }
TEST_F(SokobanMapTest, testmicroban01_0100) { testSingleMap("microban01_0100"); }
TEST_F(SokobanMapTest, testmicroban01_0101) { testSingleMap("microban01_0101"); }
TEST_F(SokobanMapTest, testmicroban01_0102) { testSingleMap("microban01_0102"); }
TEST_F(SokobanMapTest, testmicroban01_0103) { testSingleMap("microban01_0103"); }
TEST_F(SokobanMapTest, testmicroban01_0104) { testSingleMap("microban01_0104"); }
TEST_F(SokobanMapTest, testmicroban01_0105) { testSingleMap("microban01_0105"); }
TEST_F(SokobanMapTest, testmicroban01_0106) { testSingleMap("microban01_0106"); }
TEST_F(SokobanMapTest, testmicroban01_0107) { testSingleMap("microban01_0107"); }
TEST_F(SokobanMapTest, testmicroban01_0108) { testSingleMap("microban01_0108"); }
TEST_F(SokobanMapTest, testmicroban01_0109) { testSingleMap("microban01_0109"); }
TEST_F(SokobanMapTest, testmicroban01_0110) { testSingleMap("microban01_0110"); }
TEST_F(SokobanMapTest, testmicroban01_0111) { testSingleMap("microban01_0111"); }
TEST_F(SokobanMapTest, testmicroban01_0112) { testSingleMap("microban01_0112"); }
TEST_F(SokobanMapTest, testmicroban01_0113) { testSingleMap("microban01_0113"); }
TEST_F(SokobanMapTest, testmicroban01_0114) { testSingleMap("microban01_0114"); }
TEST_F(SokobanMapTest, testmicroban01_0115) { testSingleMap("microban01_0115"); }
TEST_F(SokobanMapTest, testmicroban01_0116) { testSingleMap("microban01_0116"); }
TEST_F(SokobanMapTest, testmicroban01_0117) { testSingleMap("microban01_0117"); }
TEST_F(SokobanMapTest, testmicroban01_0118) { testSingleMap("microban01_0118"); }
TEST_F(SokobanMapTest, testmicroban01_0119) { testSingleMap("microban01_0119"); }
TEST_F(SokobanMapTest, testmicroban01_0120) { testSingleMap("microban01_0120"); }
TEST_F(SokobanMapTest, testmicroban01_0121) { testSingleMap("microban01_0121"); }
TEST_F(SokobanMapTest, testmicroban01_0122) { testSingleMap("microban01_0122"); }
TEST_F(SokobanMapTest, testmicroban01_0123) { testSingleMap("microban01_0123"); }
TEST_F(SokobanMapTest, testmicroban01_0124) { testSingleMap("microban01_0124"); }
TEST_F(SokobanMapTest, testmicroban01_0125) { testSingleMap("microban01_0125"); }
TEST_F(SokobanMapTest, testmicroban01_0126) { testSingleMap("microban01_0126"); }
TEST_F(SokobanMapTest, testmicroban01_0127) { testSingleMap("microban01_0127"); }
TEST_F(SokobanMapTest, testmicroban01_0128) { testSingleMap("microban01_0128"); }
TEST_F(SokobanMapTest, testmicroban01_0129) { testSingleMap("microban01_0129"); }
TEST_F(SokobanMapTest, testmicroban01_0130) { testSingleMap("microban01_0130"); }
TEST_F(SokobanMapTest, testmicroban01_0131) { testSingleMap("microban01_0131"); }
TEST_F(SokobanMapTest, testmicroban01_0132) { testSingleMap("microban01_0132"); }
TEST_F(SokobanMapTest, testmicroban01_0133) { testSingleMap("microban01_0133"); }
TEST_F(SokobanMapTest, testmicroban01_0134) { testSingleMap("microban01_0134"); }
TEST_F(SokobanMapTest, testmicroban01_0135) { testSingleMap("microban01_0135"); }
TEST_F(SokobanMapTest, testmicroban01_0136) { testSingleMap("microban01_0136"); }
TEST_F(SokobanMapTest, testmicroban01_0137) { testSingleMap("microban01_0137"); }
TEST_F(SokobanMapTest, testmicroban01_0138) { testSingleMap("microban01_0138"); }
TEST_F(SokobanMapTest, testmicroban01_0139) { testSingleMap("microban01_0139"); }
TEST_F(SokobanMapTest, testmicroban01_0140) { testSingleMap("microban01_0140"); }
TEST_F(SokobanMapTest, testmicroban01_0141) { testSingleMap("microban01_0141"); }
TEST_F(SokobanMapTest, testmicroban01_0142) { testSingleMap("microban01_0142"); }
TEST_F(SokobanMapTest, testmicroban01_0143) { testSingleMap("microban01_0143"); }
TEST_F(SokobanMapTest, testmicroban01_0144) { testSingleMap("microban01_0144"); }
TEST_F(SokobanMapTest, testmicroban01_0145) { testSingleMap("microban01_0145"); }
TEST_F(SokobanMapTest, testmicroban01_0146) { testSingleMap("microban01_0146"); }
TEST_F(SokobanMapTest, testmicroban01_0147) { testSingleMap("microban01_0147"); }
TEST_F(SokobanMapTest, testmicroban01_0148) { testSingleMap("microban01_0148"); }
TEST_F(SokobanMapTest, testmicroban01_0149) { testSingleMap("microban01_0149"); }
TEST_F(SokobanMapTest, testmicroban01_0150) { testSingleMap("microban01_0150"); }
TEST_F(SokobanMapTest, testmicroban01_0151) { testSingleMap("microban01_0151"); }
TEST_F(SokobanMapTest, testmicroban01_0152) { testSingleMap("microban01_0152"); }
TEST_F(SokobanMapTest, testmicroban01_0153) { testSingleMap("microban01_0153"); }
TEST_F(SokobanMapTest, testmicroban01_0154) { testSingleMap("microban01_0154"); }
TEST_F(SokobanMapTest, testmicroban01_0155) { testSingleMap("microban01_0155"); }
TEST_F(SokobanMapTest, testmicroban02_0001) { testSingleMap("microban02_0001"); }
TEST_F(SokobanMapTest, testmicroban02_0002) { testSingleMap("microban02_0002"); }
TEST_F(SokobanMapTest, testmicroban02_0003) { testSingleMap("microban02_0003"); }
TEST_F(SokobanMapTest, testmicroban02_0004) { testSingleMap("microban02_0004"); }
TEST_F(SokobanMapTest, testmicroban02_0005) { testSingleMap("microban02_0005"); }
TEST_F(SokobanMapTest, testmicroban02_0006) { testSingleMap("microban02_0006"); }
TEST_F(SokobanMapTest, testmicroban02_0007) { testSingleMap("microban02_0007"); }
TEST_F(SokobanMapTest, testmicroban02_0008) { testSingleMap("microban02_0008"); }
TEST_F(SokobanMapTest, testmicroban02_0009) { testSingleMap("microban02_0009"); }
TEST_F(SokobanMapTest, testmicroban02_0010) { testSingleMap("microban02_0010"); }
TEST_F(SokobanMapTest, testmicroban02_0011) { testSingleMap("microban02_0011"); }
TEST_F(SokobanMapTest, testmicroban02_0012) { testSingleMap("microban02_0012"); }
TEST_F(SokobanMapTest, testmicroban02_0013) { testSingleMap("microban02_0013"); }
TEST_F(SokobanMapTest, testmicroban02_0014) { testSingleMap("microban02_0014"); }
TEST_F(SokobanMapTest, testmicroban02_0015) { testSingleMap("microban02_0015"); }
TEST_F(SokobanMapTest, testmicroban02_0016) { testSingleMap("microban02_0016"); }
TEST_F(SokobanMapTest, testmicroban02_0017) { testSingleMap("microban02_0017"); }
TEST_F(SokobanMapTest, testmicroban02_0018) { testSingleMap("microban02_0018"); }
TEST_F(SokobanMapTest, testmicroban02_0019) { testSingleMap("microban02_0019"); }
TEST_F(SokobanMapTest, testmicroban02_0020) { testSingleMap("microban02_0020"); }
TEST_F(SokobanMapTest, testmicroban02_0021) { testSingleMap("microban02_0021"); }
TEST_F(SokobanMapTest, testmicroban02_0022) { testSingleMap("microban02_0022"); }
TEST_F(SokobanMapTest, testmicroban02_0023) { testSingleMap("microban02_0023"); }
TEST_F(SokobanMapTest, testmicroban02_0024) { testSingleMap("microban02_0024"); }
TEST_F(SokobanMapTest, testmicroban02_0025) { testSingleMap("microban02_0025"); }
TEST_F(SokobanMapTest, testmicroban02_0026) { testSingleMap("microban02_0026"); }
TEST_F(SokobanMapTest, testmicroban02_0027) { testSingleMap("microban02_0027"); }
TEST_F(SokobanMapTest, testmicroban02_0028) { testSingleMap("microban02_0028"); }
TEST_F(SokobanMapTest, testmicroban02_0029) { testSingleMap("microban02_0029"); }
TEST_F(SokobanMapTest, testmicroban02_0030) { testSingleMap("microban02_0030"); }
TEST_F(SokobanMapTest, testmicroban02_0031) { testSingleMap("microban02_0031"); }
TEST_F(SokobanMapTest, testmicroban02_0032) { testSingleMap("microban02_0032"); }
TEST_F(SokobanMapTest, testmicroban02_0033) { testSingleMap("microban02_0033"); }
TEST_F(SokobanMapTest, testmicroban02_0034) { testSingleMap("microban02_0034"); }
TEST_F(SokobanMapTest, testmicroban02_0035) { testSingleMap("microban02_0035"); }
TEST_F(SokobanMapTest, testmicroban02_0036) { testSingleMap("microban02_0036"); }
TEST_F(SokobanMapTest, testmicroban02_0037) { testSingleMap("microban02_0037"); }
TEST_F(SokobanMapTest, testmicroban02_0038) { testSingleMap("microban02_0038"); }
TEST_F(SokobanMapTest, testmicroban02_0039) { testSingleMap("microban02_0039"); }
TEST_F(SokobanMapTest, testmicroban02_0040) { testSingleMap("microban02_0040"); }
TEST_F(SokobanMapTest, testmicroban02_0041) { testSingleMap("microban02_0041"); }
TEST_F(SokobanMapTest, testmicroban02_0042) { testSingleMap("microban02_0042"); }
TEST_F(SokobanMapTest, testmicroban02_0043) { testSingleMap("microban02_0043"); }
TEST_F(SokobanMapTest, testmicroban02_0044) { testSingleMap("microban02_0044"); }
TEST_F(SokobanMapTest, testmicroban02_0045) { testSingleMap("microban02_0045"); }
TEST_F(SokobanMapTest, testmicroban02_0046) { testSingleMap("microban02_0046"); }
TEST_F(SokobanMapTest, testmicroban02_0047) { testSingleMap("microban02_0047"); }
TEST_F(SokobanMapTest, testmicroban02_0048) { testSingleMap("microban02_0048"); }
TEST_F(SokobanMapTest, testmicroban02_0049) { testSingleMap("microban02_0049"); }
TEST_F(SokobanMapTest, testmicroban02_0050) { testSingleMap("microban02_0050"); }
TEST_F(SokobanMapTest, testmicroban02_0051) { testSingleMap("microban02_0051"); }
TEST_F(SokobanMapTest, testmicroban02_0052) { testSingleMap("microban02_0052"); }
TEST_F(SokobanMapTest, testmicroban02_0053) { testSingleMap("microban02_0053"); }
TEST_F(SokobanMapTest, testmicroban02_0054) { testSingleMap("microban02_0054"); }
TEST_F(SokobanMapTest, testmicroban02_0055) { testSingleMap("microban02_0055"); }
TEST_F(SokobanMapTest, testmicroban02_0056) { testSingleMap("microban02_0056"); }
TEST_F(SokobanMapTest, testmicroban02_0057) { testSingleMap("microban02_0057"); }
TEST_F(SokobanMapTest, testmicroban02_0058) { testSingleMap("microban02_0058"); }
TEST_F(SokobanMapTest, testmicroban02_0059) { testSingleMap("microban02_0059"); }
TEST_F(SokobanMapTest, testmicroban02_0060) { testSingleMap("microban02_0060"); }
TEST_F(SokobanMapTest, testmicroban02_0061) { testSingleMap("microban02_0061"); }
TEST_F(SokobanMapTest, testmicroban02_0062) { testSingleMap("microban02_0062"); }
TEST_F(SokobanMapTest, testmicroban02_0063) { testSingleMap("microban02_0063"); }
TEST_F(SokobanMapTest, testmicroban02_0064) { testSingleMap("microban02_0064"); }
TEST_F(SokobanMapTest, testmicroban02_0065) { testSingleMap("microban02_0065"); }
TEST_F(SokobanMapTest, testmicroban02_0066) { testSingleMap("microban02_0066"); }
TEST_F(SokobanMapTest, testmicroban02_0067) { testSingleMap("microban02_0067"); }
TEST_F(SokobanMapTest, testmicroban02_0068) { testSingleMap("microban02_0068"); }
TEST_F(SokobanMapTest, testmicroban02_0069) { testSingleMap("microban02_0069"); }
TEST_F(SokobanMapTest, testmicroban02_0070) { testSingleMap("microban02_0070"); }
TEST_F(SokobanMapTest, testmicroban02_0071) { testSingleMap("microban02_0071"); }
TEST_F(SokobanMapTest, testmicroban02_0072) { testSingleMap("microban02_0072"); }
TEST_F(SokobanMapTest, testmicroban02_0073) { testSingleMap("microban02_0073"); }
TEST_F(SokobanMapTest, testmicroban02_0074) { testSingleMap("microban02_0074"); }
TEST_F(SokobanMapTest, testmicroban02_0075) { testSingleMap("microban02_0075"); }
TEST_F(SokobanMapTest, testmicroban02_0076) { testSingleMap("microban02_0076"); }
TEST_F(SokobanMapTest, testmicroban02_0077) { testSingleMap("microban02_0077"); }
TEST_F(SokobanMapTest, testmicroban02_0078) { testSingleMap("microban02_0078"); }
TEST_F(SokobanMapTest, testmicroban02_0079) { testSingleMap("microban02_0079"); }
TEST_F(SokobanMapTest, testmicroban02_0080) { testSingleMap("microban02_0080"); }
TEST_F(SokobanMapTest, testmicroban02_0081) { testSingleMap("microban02_0081"); }
TEST_F(SokobanMapTest, testmicroban02_0082) { testSingleMap("microban02_0082"); }
TEST_F(SokobanMapTest, testmicroban02_0083) { testSingleMap("microban02_0083"); }
TEST_F(SokobanMapTest, testmicroban02_0084) { testSingleMap("microban02_0084"); }
TEST_F(SokobanMapTest, testmicroban02_0085) { testSingleMap("microban02_0085"); }
TEST_F(SokobanMapTest, testmicroban02_0086) { testSingleMap("microban02_0086"); }
TEST_F(SokobanMapTest, testmicroban02_0087) { testSingleMap("microban02_0087"); }
TEST_F(SokobanMapTest, testmicroban02_0088) { testSingleMap("microban02_0088"); }
TEST_F(SokobanMapTest, testmicroban02_0089) { testSingleMap("microban02_0089"); }
TEST_F(SokobanMapTest, testmicroban02_0090) { testSingleMap("microban02_0090"); }
TEST_F(SokobanMapTest, testmicroban02_0091) { testSingleMap("microban02_0091"); }
TEST_F(SokobanMapTest, testmicroban02_0092) { testSingleMap("microban02_0092"); }
TEST_F(SokobanMapTest, testmicroban02_0093) { testSingleMap("microban02_0093"); }
TEST_F(SokobanMapTest, testmicroban02_0094) { testSingleMap("microban02_0094"); }
TEST_F(SokobanMapTest, testmicroban02_0095) { testSingleMap("microban02_0095"); }
TEST_F(SokobanMapTest, testmicroban02_0096) { testSingleMap("microban02_0096"); }
TEST_F(SokobanMapTest, testmicroban02_0097) { testSingleMap("microban02_0097"); }
TEST_F(SokobanMapTest, testmicroban02_0098) { testSingleMap("microban02_0098"); }
TEST_F(SokobanMapTest, testmicroban02_0099) { testSingleMap("microban02_0099"); }
TEST_F(SokobanMapTest, testmicroban02_0100) { testSingleMap("microban02_0100"); }
TEST_F(SokobanMapTest, testmicroban02_0101) { testSingleMap("microban02_0101"); }
TEST_F(SokobanMapTest, testmicroban02_0102) { testSingleMap("microban02_0102"); }
TEST_F(SokobanMapTest, testmicroban02_0103) { testSingleMap("microban02_0103"); }
TEST_F(SokobanMapTest, testmicroban02_0104) { testSingleMap("microban02_0104"); }
TEST_F(SokobanMapTest, testmicroban02_0105) { testSingleMap("microban02_0105"); }
TEST_F(SokobanMapTest, testmicroban02_0106) { testSingleMap("microban02_0106"); }
TEST_F(SokobanMapTest, testmicroban02_0107) { testSingleMap("microban02_0107"); }
TEST_F(SokobanMapTest, testmicroban02_0108) { testSingleMap("microban02_0108"); }
TEST_F(SokobanMapTest, testmicroban02_0109) { testSingleMap("microban02_0109"); }
TEST_F(SokobanMapTest, testmicroban02_0110) { testSingleMap("microban02_0110"); }
TEST_F(SokobanMapTest, testmicroban02_0111) { testSingleMap("microban02_0111"); }
TEST_F(SokobanMapTest, testmicroban02_0112) { testSingleMap("microban02_0112"); }
TEST_F(SokobanMapTest, testmicroban02_0113) { testSingleMap("microban02_0113"); }
TEST_F(SokobanMapTest, testmicroban02_0114) { testSingleMap("microban02_0114"); }
TEST_F(SokobanMapTest, testmicroban02_0115) { testSingleMap("microban02_0115"); }
TEST_F(SokobanMapTest, testmicroban02_0116) { testSingleMap("microban02_0116"); }
TEST_F(SokobanMapTest, testmicroban02_0117) { testSingleMap("microban02_0117"); }
TEST_F(SokobanMapTest, testmicroban02_0118) { testSingleMap("microban02_0118"); }
TEST_F(SokobanMapTest, testmicroban02_0119) { testSingleMap("microban02_0119"); }
TEST_F(SokobanMapTest, testmicroban02_0120) { testSingleMap("microban02_0120"); }
TEST_F(SokobanMapTest, testmicroban02_0121) { testSingleMap("microban02_0121"); }
TEST_F(SokobanMapTest, testmicroban02_0122) { testSingleMap("microban02_0122"); }
TEST_F(SokobanMapTest, testmicroban02_0123) { testSingleMap("microban02_0123"); }
TEST_F(SokobanMapTest, testmicroban02_0124) { testSingleMap("microban02_0124"); }
TEST_F(SokobanMapTest, testmicroban02_0125) { testSingleMap("microban02_0125"); }
TEST_F(SokobanMapTest, testmicroban02_0126) { testSingleMap("microban02_0126"); }
TEST_F(SokobanMapTest, testmicroban02_0127) { testSingleMap("microban02_0127"); }
TEST_F(SokobanMapTest, testmicroban02_0128) { testSingleMap("microban02_0128"); }
TEST_F(SokobanMapTest, testmicroban02_0129) { testSingleMap("microban02_0129"); }
TEST_F(SokobanMapTest, testmicroban02_0130) { testSingleMap("microban02_0130"); }
TEST_F(SokobanMapTest, testmicroban02_0131) { testSingleMap("microban02_0131"); }
TEST_F(SokobanMapTest, testmicroban02_0132) { testSingleMap("microban02_0132"); }
TEST_F(SokobanMapTest, testmicroban02_0133) { testSingleMap("microban02_0133"); }
TEST_F(SokobanMapTest, testmicroban02_0134) { testSingleMap("microban02_0134"); }
TEST_F(SokobanMapTest, testmicroban02_0135) { testSingleMap("microban02_0135"); }
TEST_F(SokobanMapTest, testmicroban03_0001) { testSingleMap("microban03_0001"); }
TEST_F(SokobanMapTest, testmicroban03_0002) { testSingleMap("microban03_0002"); }
TEST_F(SokobanMapTest, testmicroban03_0003) { testSingleMap("microban03_0003"); }
TEST_F(SokobanMapTest, testmicroban03_0004) { testSingleMap("microban03_0004"); }
TEST_F(SokobanMapTest, testmicroban03_0005) { testSingleMap("microban03_0005"); }
TEST_F(SokobanMapTest, testmicroban03_0006) { testSingleMap("microban03_0006"); }
TEST_F(SokobanMapTest, testmicroban03_0007) { testSingleMap("microban03_0007"); }
TEST_F(SokobanMapTest, testmicroban03_0008) { testSingleMap("microban03_0008"); }
TEST_F(SokobanMapTest, testmicroban03_0009) { testSingleMap("microban03_0009"); }
TEST_F(SokobanMapTest, testmicroban03_0010) { testSingleMap("microban03_0010"); }
TEST_F(SokobanMapTest, testmicroban03_0011) { testSingleMap("microban03_0011"); }
TEST_F(SokobanMapTest, testmicroban03_0012) { testSingleMap("microban03_0012"); }
TEST_F(SokobanMapTest, testmicroban03_0013) { testSingleMap("microban03_0013"); }
TEST_F(SokobanMapTest, testmicroban03_0014) { testSingleMap("microban03_0014"); }
TEST_F(SokobanMapTest, testmicroban03_0015) { testSingleMap("microban03_0015"); }
TEST_F(SokobanMapTest, testmicroban03_0016) { testSingleMap("microban03_0016"); }
TEST_F(SokobanMapTest, testmicroban03_0017) { testSingleMap("microban03_0017"); }
TEST_F(SokobanMapTest, testmicroban03_0018) { testSingleMap("microban03_0018"); }
TEST_F(SokobanMapTest, testmicroban03_0019) { testSingleMap("microban03_0019"); }
TEST_F(SokobanMapTest, testmicroban03_0020) { testSingleMap("microban03_0020"); }
TEST_F(SokobanMapTest, testmicroban03_0021) { testSingleMap("microban03_0021"); }
TEST_F(SokobanMapTest, testmicroban03_0022) { testSingleMap("microban03_0022"); }
TEST_F(SokobanMapTest, testmicroban03_0023) { testSingleMap("microban03_0023"); }
TEST_F(SokobanMapTest, testmicroban03_0024) { testSingleMap("microban03_0024"); }
TEST_F(SokobanMapTest, testmicroban03_0025) { testSingleMap("microban03_0025"); }
TEST_F(SokobanMapTest, testmicroban03_0026) { testSingleMap("microban03_0026"); }
TEST_F(SokobanMapTest, testmicroban03_0027) { testSingleMap("microban03_0027"); }
TEST_F(SokobanMapTest, testmicroban03_0028) { testSingleMap("microban03_0028"); }
TEST_F(SokobanMapTest, testmicroban03_0029) { testSingleMap("microban03_0029"); }
TEST_F(SokobanMapTest, testmicroban03_0030) { testSingleMap("microban03_0030"); }
TEST_F(SokobanMapTest, testmicroban03_0031) { testSingleMap("microban03_0031"); }
TEST_F(SokobanMapTest, testmicroban03_0032) { testSingleMap("microban03_0032"); }
TEST_F(SokobanMapTest, testmicroban03_0033) { testSingleMap("microban03_0033"); }
TEST_F(SokobanMapTest, testmicroban03_0034) { testSingleMap("microban03_0034"); }
TEST_F(SokobanMapTest, testmicroban03_0035) { testSingleMap("microban03_0035"); }
TEST_F(SokobanMapTest, testmicroban03_0036) { testSingleMap("microban03_0036"); }
TEST_F(SokobanMapTest, testmicroban03_0037) { testSingleMap("microban03_0037"); }
TEST_F(SokobanMapTest, testmicroban03_0038) { testSingleMap("microban03_0038"); }
TEST_F(SokobanMapTest, testmicroban03_0039) { testSingleMap("microban03_0039"); }
TEST_F(SokobanMapTest, testmicroban03_0040) { testSingleMap("microban03_0040"); }
TEST_F(SokobanMapTest, testmicroban03_0041) { testSingleMap("microban03_0041"); }
TEST_F(SokobanMapTest, testmicroban03_0042) { testSingleMap("microban03_0042"); }
TEST_F(SokobanMapTest, testmicroban03_0043) { testSingleMap("microban03_0043"); }
TEST_F(SokobanMapTest, testmicroban03_0044) { testSingleMap("microban03_0044"); }
TEST_F(SokobanMapTest, testmicroban03_0045) { testSingleMap("microban03_0045"); }
TEST_F(SokobanMapTest, testmicroban03_0046) { testSingleMap("microban03_0046"); }
TEST_F(SokobanMapTest, testmicroban03_0047) { testSingleMap("microban03_0047"); }
TEST_F(SokobanMapTest, testmicroban03_0048) { testSingleMap("microban03_0048"); }
TEST_F(SokobanMapTest, testmicroban03_0049) { testSingleMap("microban03_0049"); }
TEST_F(SokobanMapTest, testmicroban03_0050) { testSingleMap("microban03_0050"); }
TEST_F(SokobanMapTest, testmicroban03_0051) { testSingleMap("microban03_0051"); }
TEST_F(SokobanMapTest, testmicroban03_0052) { testSingleMap("microban03_0052"); }
TEST_F(SokobanMapTest, testmicroban03_0053) { testSingleMap("microban03_0053"); }
TEST_F(SokobanMapTest, testmicroban03_0054) { testSingleMap("microban03_0054"); }
TEST_F(SokobanMapTest, testmicroban03_0055) { testSingleMap("microban03_0055"); }
TEST_F(SokobanMapTest, testmicroban03_0056) { testSingleMap("microban03_0056"); }
TEST_F(SokobanMapTest, testmicroban03_0057) { testSingleMap("microban03_0057"); }
TEST_F(SokobanMapTest, testmicroban03_0058) { testSingleMap("microban03_0058"); }
TEST_F(SokobanMapTest, testmicroban03_0059) { testSingleMap("microban03_0059"); }
TEST_F(SokobanMapTest, testmicroban03_0060) { testSingleMap("microban03_0060"); }
TEST_F(SokobanMapTest, testmicroban03_0061) { testSingleMap("microban03_0061"); }
TEST_F(SokobanMapTest, testmicroban03_0062) { testSingleMap("microban03_0062"); }
TEST_F(SokobanMapTest, testmicroban03_0063) { testSingleMap("microban03_0063"); }
TEST_F(SokobanMapTest, testmicroban03_0064) { testSingleMap("microban03_0064"); }
TEST_F(SokobanMapTest, testmicroban03_0065) { testSingleMap("microban03_0065"); }
TEST_F(SokobanMapTest, testmicroban03_0066) { testSingleMap("microban03_0066"); }
TEST_F(SokobanMapTest, testmicroban03_0067) { testSingleMap("microban03_0067"); }
TEST_F(SokobanMapTest, testmicroban03_0068) { testSingleMap("microban03_0068"); }
TEST_F(SokobanMapTest, testmicroban03_0069) { testSingleMap("microban03_0069"); }
TEST_F(SokobanMapTest, testmicroban03_0070) { testSingleMap("microban03_0070"); }
TEST_F(SokobanMapTest, testmicroban03_0071) { testSingleMap("microban03_0071"); }
TEST_F(SokobanMapTest, testmicroban03_0072) { testSingleMap("microban03_0072"); }
TEST_F(SokobanMapTest, testmicroban03_0073) { testSingleMap("microban03_0073"); }
TEST_F(SokobanMapTest, testmicroban03_0074) { testSingleMap("microban03_0074"); }
TEST_F(SokobanMapTest, testmicroban03_0075) { testSingleMap("microban03_0075"); }
TEST_F(SokobanMapTest, testmicroban03_0076) { testSingleMap("microban03_0076"); }
TEST_F(SokobanMapTest, testmicroban03_0077) { testSingleMap("microban03_0077"); }
TEST_F(SokobanMapTest, testmicroban03_0078) { testSingleMap("microban03_0078"); }
TEST_F(SokobanMapTest, testmicroban03_0079) { testSingleMap("microban03_0079"); }
TEST_F(SokobanMapTest, testmicroban03_0080) { testSingleMap("microban03_0080"); }
TEST_F(SokobanMapTest, testmicroban03_0081) { testSingleMap("microban03_0081"); }
TEST_F(SokobanMapTest, testmicroban03_0082) { testSingleMap("microban03_0082"); }
TEST_F(SokobanMapTest, testmicroban03_0083) { testSingleMap("microban03_0083"); }
TEST_F(SokobanMapTest, testmicroban03_0084) { testSingleMap("microban03_0084"); }
TEST_F(SokobanMapTest, testmicroban03_0085) { testSingleMap("microban03_0085"); }
TEST_F(SokobanMapTest, testmicroban03_0086) { testSingleMap("microban03_0086"); }
TEST_F(SokobanMapTest, testmicroban03_0087) { testSingleMap("microban03_0087"); }
TEST_F(SokobanMapTest, testmicroban03_0088) { testSingleMap("microban03_0088"); }
TEST_F(SokobanMapTest, testmicroban03_0089) { testSingleMap("microban03_0089"); }
TEST_F(SokobanMapTest, testmicroban03_0090) { testSingleMap("microban03_0090"); }
TEST_F(SokobanMapTest, testmicroban03_0091) { testSingleMap("microban03_0091"); }
TEST_F(SokobanMapTest, testmicroban03_0092) { testSingleMap("microban03_0092"); }
TEST_F(SokobanMapTest, testmicroban03_0093) { testSingleMap("microban03_0093"); }
TEST_F(SokobanMapTest, testmicroban03_0094) { testSingleMap("microban03_0094"); }
TEST_F(SokobanMapTest, testmicroban03_0095) { testSingleMap("microban03_0095"); }
TEST_F(SokobanMapTest, testmicroban03_0096) { testSingleMap("microban03_0096"); }
TEST_F(SokobanMapTest, testmicroban03_0097) { testSingleMap("microban03_0097"); }
TEST_F(SokobanMapTest, testmicroban03_0098) { testSingleMap("microban03_0098"); }
TEST_F(SokobanMapTest, testmicroban03_0099) { testSingleMap("microban03_0099"); }
TEST_F(SokobanMapTest, testmicroban03_0100) { testSingleMap("microban03_0100"); }
TEST_F(SokobanMapTest, testmicroban03_0101) { testSingleMap("microban03_0101"); }
TEST_F(SokobanMapTest, testmicroban04_0001) { testSingleMap("microban04_0001"); }
TEST_F(SokobanMapTest, testmicroban04_0002) { testSingleMap("microban04_0002"); }
TEST_F(SokobanMapTest, testmicroban04_0003) { testSingleMap("microban04_0003"); }
TEST_F(SokobanMapTest, testmicroban04_0004) { testSingleMap("microban04_0004"); }
TEST_F(SokobanMapTest, testmicroban04_0005) { testSingleMap("microban04_0005"); }
TEST_F(SokobanMapTest, testmicroban04_0006) { testSingleMap("microban04_0006"); }
TEST_F(SokobanMapTest, testmicroban04_0007) { testSingleMap("microban04_0007"); }
TEST_F(SokobanMapTest, testmicroban04_0008) { testSingleMap("microban04_0008"); }
TEST_F(SokobanMapTest, testmicroban04_0009) { testSingleMap("microban04_0009"); }
TEST_F(SokobanMapTest, testmicroban04_0010) { testSingleMap("microban04_0010"); }
TEST_F(SokobanMapTest, testmicroban04_0011) { testSingleMap("microban04_0011"); }
TEST_F(SokobanMapTest, testmicroban04_0012) { testSingleMap("microban04_0012"); }
TEST_F(SokobanMapTest, testmicroban04_0013) { testSingleMap("microban04_0013"); }
TEST_F(SokobanMapTest, testmicroban04_0014) { testSingleMap("microban04_0014"); }
TEST_F(SokobanMapTest, testmicroban04_0015) { testSingleMap("microban04_0015"); }
TEST_F(SokobanMapTest, testmicroban04_0016) { testSingleMap("microban04_0016"); }
TEST_F(SokobanMapTest, testmicroban04_0017) { testSingleMap("microban04_0017"); }
TEST_F(SokobanMapTest, testmicroban04_0018) { testSingleMap("microban04_0018"); }
TEST_F(SokobanMapTest, testmicroban04_0019) { testSingleMap("microban04_0019"); }
TEST_F(SokobanMapTest, testmicroban04_0020) { testSingleMap("microban04_0020"); }
TEST_F(SokobanMapTest, testmicroban04_0021) { testSingleMap("microban04_0021"); }
TEST_F(SokobanMapTest, testmicroban04_0022) { testSingleMap("microban04_0022"); }
TEST_F(SokobanMapTest, testmicroban04_0023) { testSingleMap("microban04_0023"); }
TEST_F(SokobanMapTest, testmicroban04_0024) { testSingleMap("microban04_0024"); }
TEST_F(SokobanMapTest, testmicroban04_0025) { testSingleMap("microban04_0025"); }
TEST_F(SokobanMapTest, testmicroban04_0026) { testSingleMap("microban04_0026"); }
TEST_F(SokobanMapTest, testmicroban04_0027) { testSingleMap("microban04_0027"); }
TEST_F(SokobanMapTest, testmicroban04_0028) { testSingleMap("microban04_0028"); }
TEST_F(SokobanMapTest, testmicroban04_0029) { testSingleMap("microban04_0029"); }
TEST_F(SokobanMapTest, testmicroban04_0030) { testSingleMap("microban04_0030"); }
TEST_F(SokobanMapTest, testmicroban04_0031) { testSingleMap("microban04_0031"); }
TEST_F(SokobanMapTest, testmicroban04_0032) { testSingleMap("microban04_0032"); }
TEST_F(SokobanMapTest, testmicroban04_0033) { testSingleMap("microban04_0033"); }
TEST_F(SokobanMapTest, testmicroban04_0034) { testSingleMap("microban04_0034"); }
TEST_F(SokobanMapTest, testmicroban04_0035) { testSingleMap("microban04_0035"); }
TEST_F(SokobanMapTest, testmicroban04_0036) { testSingleMap("microban04_0036"); }
TEST_F(SokobanMapTest, testmicroban04_0037) { testSingleMap("microban04_0037"); }
TEST_F(SokobanMapTest, testmicroban04_0038) { testSingleMap("microban04_0038"); }
TEST_F(SokobanMapTest, testmicroban04_0039) { testSingleMap("microban04_0039"); }
TEST_F(SokobanMapTest, testmicroban04_0040) { testSingleMap("microban04_0040"); }
TEST_F(SokobanMapTest, testmicroban04_0041) { testSingleMap("microban04_0041"); }
TEST_F(SokobanMapTest, testmicroban04_0042) { testSingleMap("microban04_0042"); }
TEST_F(SokobanMapTest, testmicroban04_0043) { testSingleMap("microban04_0043"); }
TEST_F(SokobanMapTest, testmicroban04_0044) { testSingleMap("microban04_0044"); }
TEST_F(SokobanMapTest, testmicroban04_0045) { testSingleMap("microban04_0045"); }
TEST_F(SokobanMapTest, testmicroban04_0046) { testSingleMap("microban04_0046"); }
TEST_F(SokobanMapTest, testmicroban04_0047) { testSingleMap("microban04_0047"); }
TEST_F(SokobanMapTest, testmicroban04_0048) { testSingleMap("microban04_0048"); }
TEST_F(SokobanMapTest, testmicroban04_0049) { testSingleMap("microban04_0049"); }
TEST_F(SokobanMapTest, testmicroban04_0050) { testSingleMap("microban04_0050"); }
TEST_F(SokobanMapTest, testmicroban04_0051) { testSingleMap("microban04_0051"); }
TEST_F(SokobanMapTest, testmicroban04_0052) { testSingleMap("microban04_0052"); }
TEST_F(SokobanMapTest, testmicroban04_0053) { testSingleMap("microban04_0053"); }
TEST_F(SokobanMapTest, testmicroban04_0054) { testSingleMap("microban04_0054"); }
TEST_F(SokobanMapTest, testmicroban04_0055) { testSingleMap("microban04_0055"); }
TEST_F(SokobanMapTest, testmicroban04_0056) { testSingleMap("microban04_0056"); }
TEST_F(SokobanMapTest, testmicroban04_0057) { testSingleMap("microban04_0057"); }
TEST_F(SokobanMapTest, testmicroban04_0058) { testSingleMap("microban04_0058"); }
TEST_F(SokobanMapTest, testmicroban04_0059) { testSingleMap("microban04_0059"); }
TEST_F(SokobanMapTest, testmicroban04_0060) { testSingleMap("microban04_0060"); }
TEST_F(SokobanMapTest, testmicroban04_0061) { testSingleMap("microban04_0061"); }
TEST_F(SokobanMapTest, testmicroban04_0062) { testSingleMap("microban04_0062"); }
TEST_F(SokobanMapTest, testmicroban04_0063) { testSingleMap("microban04_0063"); }
TEST_F(SokobanMapTest, testmicroban04_0064) { testSingleMap("microban04_0064"); }
TEST_F(SokobanMapTest, testmicroban04_0065) { testSingleMap("microban04_0065"); }
TEST_F(SokobanMapTest, testmicroban04_0066) { testSingleMap("microban04_0066"); }
TEST_F(SokobanMapTest, testmicroban04_0067) { testSingleMap("microban04_0067"); }
TEST_F(SokobanMapTest, testmicroban04_0068) { testSingleMap("microban04_0068"); }
TEST_F(SokobanMapTest, testmicroban04_0069) { testSingleMap("microban04_0069"); }
TEST_F(SokobanMapTest, testmicroban04_0070) { testSingleMap("microban04_0070"); }
TEST_F(SokobanMapTest, testmicroban04_0071) { testSingleMap("microban04_0071"); }
TEST_F(SokobanMapTest, testmicroban04_0072) { testSingleMap("microban04_0072"); }
TEST_F(SokobanMapTest, testmicroban04_0073) { testSingleMap("microban04_0073"); }
TEST_F(SokobanMapTest, testmicroban04_0074) { testSingleMap("microban04_0074"); }
TEST_F(SokobanMapTest, testmicroban04_0075) { testSingleMap("microban04_0075"); }
TEST_F(SokobanMapTest, testmicroban04_0076) { testSingleMap("microban04_0076"); }
TEST_F(SokobanMapTest, testmicroban04_0077) { testSingleMap("microban04_0077"); }
TEST_F(SokobanMapTest, testmicroban04_0078) { testSingleMap("microban04_0078"); }
TEST_F(SokobanMapTest, testmicroban04_0079) { testSingleMap("microban04_0079"); }
TEST_F(SokobanMapTest, testmicroban04_0080) { testSingleMap("microban04_0080"); }
TEST_F(SokobanMapTest, testmicroban04_0081) { testSingleMap("microban04_0081"); }
TEST_F(SokobanMapTest, testmicroban04_0082) { testSingleMap("microban04_0082"); }
TEST_F(SokobanMapTest, testmicroban04_0083) { testSingleMap("microban04_0083"); }
TEST_F(SokobanMapTest, testmicroban04_0084) { testSingleMap("microban04_0084"); }
TEST_F(SokobanMapTest, testmicroban04_0085) { testSingleMap("microban04_0085"); }
TEST_F(SokobanMapTest, testmicroban04_0086) { testSingleMap("microban04_0086"); }
TEST_F(SokobanMapTest, testmicroban04_0087) { testSingleMap("microban04_0087"); }
TEST_F(SokobanMapTest, testmicroban04_0088) { testSingleMap("microban04_0088"); }
TEST_F(SokobanMapTest, testmicroban04_0089) { testSingleMap("microban04_0089"); }
TEST_F(SokobanMapTest, testmicroban04_0090) { testSingleMap("microban04_0090"); }
TEST_F(SokobanMapTest, testmicroban04_0091) { testSingleMap("microban04_0091"); }
TEST_F(SokobanMapTest, testmicroban04_0092) { testSingleMap("microban04_0092"); }
TEST_F(SokobanMapTest, testmicroban04_0093) { testSingleMap("microban04_0093"); }
TEST_F(SokobanMapTest, testmicroban04_0094) { testSingleMap("microban04_0094"); }
TEST_F(SokobanMapTest, testmicroban04_0095) { testSingleMap("microban04_0095"); }
TEST_F(SokobanMapTest, testmicroban04_0096) { testSingleMap("microban04_0096"); }
TEST_F(SokobanMapTest, testmicroban04_0097) { testSingleMap("microban04_0097"); }
TEST_F(SokobanMapTest, testmicroban04_0098) { testSingleMap("microban04_0098"); }
TEST_F(SokobanMapTest, testmicroban04_0099) { testSingleMap("microban04_0099"); }
TEST_F(SokobanMapTest, testmicroban04_0100) { testSingleMap("microban04_0100"); }
TEST_F(SokobanMapTest, testmicroban04_0101) { testSingleMap("microban04_0101"); }
TEST_F(SokobanMapTest, testmicroban04_0102) { testSingleMap("microban04_0102"); }
TEST_F(SokobanMapTest, testsasquatch01_0001) { testSingleMap("sasquatch01_0001"); }
TEST_F(SokobanMapTest, testsasquatch01_0002) { testSingleMap("sasquatch01_0002"); }
TEST_F(SokobanMapTest, testsasquatch01_0003) { testSingleMap("sasquatch01_0003"); }
TEST_F(SokobanMapTest, testsasquatch01_0004) { testSingleMap("sasquatch01_0004"); }
TEST_F(SokobanMapTest, testsasquatch01_0005) { testSingleMap("sasquatch01_0005"); }
TEST_F(SokobanMapTest, testsasquatch01_0006) { testSingleMap("sasquatch01_0006"); }
TEST_F(SokobanMapTest, testsasquatch01_0007) { testSingleMap("sasquatch01_0007"); }
TEST_F(SokobanMapTest, testsasquatch01_0008) { testSingleMap("sasquatch01_0008"); }
TEST_F(SokobanMapTest, testsasquatch01_0009) { testSingleMap("sasquatch01_0009"); }
TEST_F(SokobanMapTest, testsasquatch01_0010) { testSingleMap("sasquatch01_0010"); }
TEST_F(SokobanMapTest, testsasquatch01_0011) { testSingleMap("sasquatch01_0011"); }
TEST_F(SokobanMapTest, testsasquatch01_0012) { testSingleMap("sasquatch01_0012"); }
TEST_F(SokobanMapTest, testsasquatch01_0013) { testSingleMap("sasquatch01_0013"); }
TEST_F(SokobanMapTest, testsasquatch01_0014) { testSingleMap("sasquatch01_0014"); }
TEST_F(SokobanMapTest, testsasquatch01_0015) { testSingleMap("sasquatch01_0015"); }
TEST_F(SokobanMapTest, testsasquatch01_0016) { testSingleMap("sasquatch01_0016"); }
TEST_F(SokobanMapTest, testsasquatch01_0017) { testSingleMap("sasquatch01_0017"); }
TEST_F(SokobanMapTest, testsasquatch01_0018) { testSingleMap("sasquatch01_0018"); }
TEST_F(SokobanMapTest, testsasquatch01_0019) { testSingleMap("sasquatch01_0019"); }
TEST_F(SokobanMapTest, testsasquatch01_0020) { testSingleMap("sasquatch01_0020"); }
TEST_F(SokobanMapTest, testsasquatch01_0021) { testSingleMap("sasquatch01_0021"); }
TEST_F(SokobanMapTest, testsasquatch01_0022) { testSingleMap("sasquatch01_0022"); }
TEST_F(SokobanMapTest, testsasquatch01_0023) { testSingleMap("sasquatch01_0023"); }
TEST_F(SokobanMapTest, testsasquatch01_0024) { testSingleMap("sasquatch01_0024"); }
TEST_F(SokobanMapTest, testsasquatch01_0025) { testSingleMap("sasquatch01_0025"); }
TEST_F(SokobanMapTest, testsasquatch01_0026) { testSingleMap("sasquatch01_0026"); }
TEST_F(SokobanMapTest, testsasquatch01_0027) { testSingleMap("sasquatch01_0027"); }
TEST_F(SokobanMapTest, testsasquatch01_0028) { testSingleMap("sasquatch01_0028"); }
TEST_F(SokobanMapTest, testsasquatch01_0029) { testSingleMap("sasquatch01_0029"); }
TEST_F(SokobanMapTest, testsasquatch01_0030) { testSingleMap("sasquatch01_0030"); }
TEST_F(SokobanMapTest, testsasquatch01_0031) { testSingleMap("sasquatch01_0031"); }
TEST_F(SokobanMapTest, testsasquatch01_0032) { testSingleMap("sasquatch01_0032"); }
TEST_F(SokobanMapTest, testsasquatch01_0033) { testSingleMap("sasquatch01_0033"); }
TEST_F(SokobanMapTest, testsasquatch01_0034) { testSingleMap("sasquatch01_0034"); }
TEST_F(SokobanMapTest, testsasquatch01_0035) { testSingleMap("sasquatch01_0035"); }
TEST_F(SokobanMapTest, testsasquatch01_0036) { testSingleMap("sasquatch01_0036"); }
TEST_F(SokobanMapTest, testsasquatch01_0037) { testSingleMap("sasquatch01_0037"); }
TEST_F(SokobanMapTest, testsasquatch01_0038) { testSingleMap("sasquatch01_0038"); }
TEST_F(SokobanMapTest, testsasquatch01_0039) { testSingleMap("sasquatch01_0039"); }
TEST_F(SokobanMapTest, testsasquatch01_0040) { testSingleMap("sasquatch01_0040"); }
TEST_F(SokobanMapTest, testsasquatch01_0041) { testSingleMap("sasquatch01_0041"); }
TEST_F(SokobanMapTest, testsasquatch01_0042) { testSingleMap("sasquatch01_0042"); }
TEST_F(SokobanMapTest, testsasquatch01_0043) { testSingleMap("sasquatch01_0043"); }
TEST_F(SokobanMapTest, testsasquatch01_0044) { testSingleMap("sasquatch01_0044"); }
TEST_F(SokobanMapTest, testsasquatch01_0045) { testSingleMap("sasquatch01_0045"); }
TEST_F(SokobanMapTest, testsasquatch01_0046) { testSingleMap("sasquatch01_0046"); }
TEST_F(SokobanMapTest, testsasquatch01_0047) { testSingleMap("sasquatch01_0047"); }
TEST_F(SokobanMapTest, testsasquatch01_0048) { testSingleMap("sasquatch01_0048"); }
TEST_F(SokobanMapTest, testsasquatch01_0049) { testSingleMap("sasquatch01_0049"); }
TEST_F(SokobanMapTest, testsasquatch01_0050) { testSingleMap("sasquatch01_0050"); }
TEST_F(SokobanMapTest, testsasquatch02_0001) { testSingleMap("sasquatch02_0001"); }
TEST_F(SokobanMapTest, testsasquatch02_0002) { testSingleMap("sasquatch02_0002"); }
TEST_F(SokobanMapTest, testsasquatch02_0003) { testSingleMap("sasquatch02_0003"); }
TEST_F(SokobanMapTest, testsasquatch02_0004) { testSingleMap("sasquatch02_0004"); }
TEST_F(SokobanMapTest, testsasquatch02_0005) { testSingleMap("sasquatch02_0005"); }
TEST_F(SokobanMapTest, testsasquatch02_0006) { testSingleMap("sasquatch02_0006"); }
TEST_F(SokobanMapTest, testsasquatch02_0007) { testSingleMap("sasquatch02_0007"); }
TEST_F(SokobanMapTest, testsasquatch02_0008) { testSingleMap("sasquatch02_0008"); }
TEST_F(SokobanMapTest, testsasquatch02_0009) { testSingleMap("sasquatch02_0009"); }
TEST_F(SokobanMapTest, testsasquatch02_0010) { testSingleMap("sasquatch02_0010"); }
TEST_F(SokobanMapTest, testsasquatch02_0011) { testSingleMap("sasquatch02_0011"); }
TEST_F(SokobanMapTest, testsasquatch02_0012) { testSingleMap("sasquatch02_0012"); }
TEST_F(SokobanMapTest, testsasquatch02_0013) { testSingleMap("sasquatch02_0013"); }
TEST_F(SokobanMapTest, testsasquatch02_0014) { testSingleMap("sasquatch02_0014"); }
TEST_F(SokobanMapTest, testsasquatch02_0015) { testSingleMap("sasquatch02_0015"); }
TEST_F(SokobanMapTest, testsasquatch02_0016) { testSingleMap("sasquatch02_0016"); }
TEST_F(SokobanMapTest, testsasquatch02_0017) { testSingleMap("sasquatch02_0017"); }
TEST_F(SokobanMapTest, testsasquatch02_0018) { testSingleMap("sasquatch02_0018"); }
TEST_F(SokobanMapTest, testsasquatch02_0019) { testSingleMap("sasquatch02_0019"); }
TEST_F(SokobanMapTest, testsasquatch02_0020) { testSingleMap("sasquatch02_0020"); }
TEST_F(SokobanMapTest, testsasquatch02_0021) { testSingleMap("sasquatch02_0021"); }
TEST_F(SokobanMapTest, testsasquatch02_0022) { testSingleMap("sasquatch02_0022"); }
TEST_F(SokobanMapTest, testsasquatch02_0023) { testSingleMap("sasquatch02_0023"); }
TEST_F(SokobanMapTest, testsasquatch02_0024) { testSingleMap("sasquatch02_0024"); }
TEST_F(SokobanMapTest, testsasquatch02_0025) { testSingleMap("sasquatch02_0025"); }
TEST_F(SokobanMapTest, testsasquatch02_0026) { testSingleMap("sasquatch02_0026"); }
TEST_F(SokobanMapTest, testsasquatch02_0027) { testSingleMap("sasquatch02_0027"); }
TEST_F(SokobanMapTest, testsasquatch02_0028) { testSingleMap("sasquatch02_0028"); }
TEST_F(SokobanMapTest, testsasquatch02_0029) { testSingleMap("sasquatch02_0029"); }
TEST_F(SokobanMapTest, testsasquatch02_0030) { testSingleMap("sasquatch02_0030"); }
TEST_F(SokobanMapTest, testsasquatch02_0031) { testSingleMap("sasquatch02_0031"); }
TEST_F(SokobanMapTest, testsasquatch02_0032) { testSingleMap("sasquatch02_0032"); }
TEST_F(SokobanMapTest, testsasquatch02_0033) { testSingleMap("sasquatch02_0033"); }
TEST_F(SokobanMapTest, testsasquatch02_0034) { testSingleMap("sasquatch02_0034"); }
TEST_F(SokobanMapTest, testsasquatch02_0035) { testSingleMap("sasquatch02_0035"); }
TEST_F(SokobanMapTest, testsasquatch02_0036) { testSingleMap("sasquatch02_0036"); }
TEST_F(SokobanMapTest, testsasquatch02_0037) { testSingleMap("sasquatch02_0037"); }
TEST_F(SokobanMapTest, testsasquatch02_0038) { testSingleMap("sasquatch02_0038"); }
TEST_F(SokobanMapTest, testsasquatch02_0039) { testSingleMap("sasquatch02_0039"); }
TEST_F(SokobanMapTest, testsasquatch02_0040) { testSingleMap("sasquatch02_0040"); }
TEST_F(SokobanMapTest, testsasquatch02_0041) { testSingleMap("sasquatch02_0041"); }
TEST_F(SokobanMapTest, testsasquatch02_0042) { testSingleMap("sasquatch02_0042"); }
TEST_F(SokobanMapTest, testsasquatch02_0043) { testSingleMap("sasquatch02_0043"); }
TEST_F(SokobanMapTest, testsasquatch02_0044) { testSingleMap("sasquatch02_0044"); }
TEST_F(SokobanMapTest, testsasquatch02_0045) { testSingleMap("sasquatch02_0045"); }
TEST_F(SokobanMapTest, testsasquatch02_0046) { testSingleMap("sasquatch02_0046"); }
TEST_F(SokobanMapTest, testsasquatch02_0047) { testSingleMap("sasquatch02_0047"); }
TEST_F(SokobanMapTest, testsasquatch02_0048) { testSingleMap("sasquatch02_0048"); }
TEST_F(SokobanMapTest, testsasquatch02_0049) { testSingleMap("sasquatch02_0049"); }
TEST_F(SokobanMapTest, testsasquatch02_0050) { testSingleMap("sasquatch02_0050"); }
TEST_F(SokobanMapTest, testsasquatch03_0001) { testSingleMap("sasquatch03_0001"); }
TEST_F(SokobanMapTest, testsasquatch03_0002) { testSingleMap("sasquatch03_0002"); }
TEST_F(SokobanMapTest, testsasquatch03_0003) { testSingleMap("sasquatch03_0003"); }
TEST_F(SokobanMapTest, testsasquatch03_0004) { testSingleMap("sasquatch03_0004"); }
TEST_F(SokobanMapTest, testsasquatch03_0005) { testSingleMap("sasquatch03_0005"); }
TEST_F(SokobanMapTest, testsasquatch03_0006) { testSingleMap("sasquatch03_0006"); }
TEST_F(SokobanMapTest, testsasquatch03_0007) { testSingleMap("sasquatch03_0007"); }
TEST_F(SokobanMapTest, testsasquatch03_0008) { testSingleMap("sasquatch03_0008"); }
TEST_F(SokobanMapTest, testsasquatch03_0009) { testSingleMap("sasquatch03_0009"); }
TEST_F(SokobanMapTest, testsasquatch03_0010) { testSingleMap("sasquatch03_0010"); }
TEST_F(SokobanMapTest, testsasquatch03_0011) { testSingleMap("sasquatch03_0011"); }
TEST_F(SokobanMapTest, testsasquatch03_0012) { testSingleMap("sasquatch03_0012"); }
TEST_F(SokobanMapTest, testsasquatch03_0013) { testSingleMap("sasquatch03_0013"); }
TEST_F(SokobanMapTest, testsasquatch03_0014) { testSingleMap("sasquatch03_0014"); }
TEST_F(SokobanMapTest, testsasquatch03_0015) { testSingleMap("sasquatch03_0015"); }
TEST_F(SokobanMapTest, testsasquatch03_0016) { testSingleMap("sasquatch03_0016"); }
TEST_F(SokobanMapTest, testsasquatch03_0017) { testSingleMap("sasquatch03_0017"); }
TEST_F(SokobanMapTest, testsasquatch03_0018) { testSingleMap("sasquatch03_0018"); }
TEST_F(SokobanMapTest, testsasquatch03_0019) { testSingleMap("sasquatch03_0019"); }
TEST_F(SokobanMapTest, testsasquatch03_0020) { testSingleMap("sasquatch03_0020"); }
TEST_F(SokobanMapTest, testsasquatch03_0021) { testSingleMap("sasquatch03_0021"); }
TEST_F(SokobanMapTest, testsasquatch03_0022) { testSingleMap("sasquatch03_0022"); }
TEST_F(SokobanMapTest, testsasquatch03_0023) { testSingleMap("sasquatch03_0023"); }
TEST_F(SokobanMapTest, testsasquatch03_0024) { testSingleMap("sasquatch03_0024"); }
TEST_F(SokobanMapTest, testsasquatch03_0025) { testSingleMap("sasquatch03_0025"); }
TEST_F(SokobanMapTest, testsasquatch03_0026) { testSingleMap("sasquatch03_0026"); }
TEST_F(SokobanMapTest, testsasquatch03_0027) { testSingleMap("sasquatch03_0027"); }
TEST_F(SokobanMapTest, testsasquatch03_0028) { testSingleMap("sasquatch03_0028"); }
TEST_F(SokobanMapTest, testsasquatch03_0029) { testSingleMap("sasquatch03_0029"); }
TEST_F(SokobanMapTest, testsasquatch03_0030) { testSingleMap("sasquatch03_0030"); }
TEST_F(SokobanMapTest, testsasquatch03_0031) { testSingleMap("sasquatch03_0031"); }
TEST_F(SokobanMapTest, testsasquatch03_0032) { testSingleMap("sasquatch03_0032"); }
TEST_F(SokobanMapTest, testsasquatch03_0033) { testSingleMap("sasquatch03_0033"); }
TEST_F(SokobanMapTest, testsasquatch03_0034) { testSingleMap("sasquatch03_0034"); }
TEST_F(SokobanMapTest, testsasquatch03_0035) { testSingleMap("sasquatch03_0035"); }
TEST_F(SokobanMapTest, testsasquatch03_0036) { testSingleMap("sasquatch03_0036"); }
TEST_F(SokobanMapTest, testsasquatch03_0037) { testSingleMap("sasquatch03_0037"); }
TEST_F(SokobanMapTest, testsasquatch03_0038) { testSingleMap("sasquatch03_0038"); }
TEST_F(SokobanMapTest, testsasquatch03_0039) { testSingleMap("sasquatch03_0039"); }
TEST_F(SokobanMapTest, testsasquatch03_0040) { testSingleMap("sasquatch03_0040"); }
TEST_F(SokobanMapTest, testsasquatch03_0041) { testSingleMap("sasquatch03_0041"); }
TEST_F(SokobanMapTest, testsasquatch03_0042) { testSingleMap("sasquatch03_0042"); }
TEST_F(SokobanMapTest, testsasquatch03_0043) { testSingleMap("sasquatch03_0043"); }
TEST_F(SokobanMapTest, testsasquatch03_0044) { testSingleMap("sasquatch03_0044"); }
TEST_F(SokobanMapTest, testsasquatch03_0045) { testSingleMap("sasquatch03_0045"); }
TEST_F(SokobanMapTest, testsasquatch03_0046) { testSingleMap("sasquatch03_0046"); }
TEST_F(SokobanMapTest, testsasquatch03_0047) { testSingleMap("sasquatch03_0047"); }
TEST_F(SokobanMapTest, testsasquatch03_0048) { testSingleMap("sasquatch03_0048"); }
TEST_F(SokobanMapTest, testsasquatch03_0049) { testSingleMap("sasquatch03_0049"); }
TEST_F(SokobanMapTest, testsasquatch03_0050) { testSingleMap("sasquatch03_0050"); }
TEST_F(SokobanMapTest, testsasquatch04_0001) { testSingleMap("sasquatch04_0001"); }
TEST_F(SokobanMapTest, testsasquatch04_0002) { testSingleMap("sasquatch04_0002"); }
TEST_F(SokobanMapTest, testsasquatch04_0003) { testSingleMap("sasquatch04_0003"); }
TEST_F(SokobanMapTest, testsasquatch04_0004) { testSingleMap("sasquatch04_0004"); }
TEST_F(SokobanMapTest, testsasquatch04_0005) { testSingleMap("sasquatch04_0005"); }
TEST_F(SokobanMapTest, testsasquatch04_0006) { testSingleMap("sasquatch04_0006"); }
TEST_F(SokobanMapTest, testsasquatch04_0007) { testSingleMap("sasquatch04_0007"); }
TEST_F(SokobanMapTest, testsasquatch04_0008) { testSingleMap("sasquatch04_0008"); }
TEST_F(SokobanMapTest, testsasquatch04_0009) { testSingleMap("sasquatch04_0009"); }
TEST_F(SokobanMapTest, testsasquatch04_0010) { testSingleMap("sasquatch04_0010"); }
TEST_F(SokobanMapTest, testsasquatch04_0011) { testSingleMap("sasquatch04_0011"); }
TEST_F(SokobanMapTest, testsasquatch04_0012) { testSingleMap("sasquatch04_0012"); }
TEST_F(SokobanMapTest, testsasquatch04_0013) { testSingleMap("sasquatch04_0013"); }
TEST_F(SokobanMapTest, testsasquatch04_0014) { testSingleMap("sasquatch04_0014"); }
TEST_F(SokobanMapTest, testsasquatch04_0015) { testSingleMap("sasquatch04_0015"); }
TEST_F(SokobanMapTest, testsasquatch04_0016) { testSingleMap("sasquatch04_0016"); }
TEST_F(SokobanMapTest, testsasquatch04_0017) { testSingleMap("sasquatch04_0017"); }
TEST_F(SokobanMapTest, testsasquatch04_0018) { testSingleMap("sasquatch04_0018"); }
TEST_F(SokobanMapTest, testsasquatch04_0019) { testSingleMap("sasquatch04_0019"); }
TEST_F(SokobanMapTest, testsasquatch04_0020) { testSingleMap("sasquatch04_0020"); }
TEST_F(SokobanMapTest, testsasquatch04_0021) { testSingleMap("sasquatch04_0021"); }
TEST_F(SokobanMapTest, testsasquatch04_0022) { testSingleMap("sasquatch04_0022"); }
TEST_F(SokobanMapTest, testsasquatch04_0023) { testSingleMap("sasquatch04_0023"); }
TEST_F(SokobanMapTest, testsasquatch04_0024) { testSingleMap("sasquatch04_0024"); }
TEST_F(SokobanMapTest, testsasquatch04_0025) { testSingleMap("sasquatch04_0025"); }
TEST_F(SokobanMapTest, testsasquatch04_0026) { testSingleMap("sasquatch04_0026"); }
TEST_F(SokobanMapTest, testsasquatch04_0027) { testSingleMap("sasquatch04_0027"); }
TEST_F(SokobanMapTest, testsasquatch04_0028) { testSingleMap("sasquatch04_0028"); }
TEST_F(SokobanMapTest, testsasquatch04_0029) { testSingleMap("sasquatch04_0029"); }
TEST_F(SokobanMapTest, testsasquatch04_0030) { testSingleMap("sasquatch04_0030"); }
TEST_F(SokobanMapTest, testsasquatch04_0031) { testSingleMap("sasquatch04_0031"); }
TEST_F(SokobanMapTest, testsasquatch04_0032) { testSingleMap("sasquatch04_0032"); }
TEST_F(SokobanMapTest, testsasquatch04_0033) { testSingleMap("sasquatch04_0033"); }
TEST_F(SokobanMapTest, testsasquatch04_0034) { testSingleMap("sasquatch04_0034"); }
TEST_F(SokobanMapTest, testsasquatch04_0035) { testSingleMap("sasquatch04_0035"); }
TEST_F(SokobanMapTest, testsasquatch04_0036) { testSingleMap("sasquatch04_0036"); }
TEST_F(SokobanMapTest, testsasquatch04_0037) { testSingleMap("sasquatch04_0037"); }
TEST_F(SokobanMapTest, testsasquatch04_0038) { testSingleMap("sasquatch04_0038"); }
TEST_F(SokobanMapTest, testsasquatch04_0039) { testSingleMap("sasquatch04_0039"); }
TEST_F(SokobanMapTest, testsasquatch04_0040) { testSingleMap("sasquatch04_0040"); }
TEST_F(SokobanMapTest, testsasquatch04_0041) { testSingleMap("sasquatch04_0041"); }
TEST_F(SokobanMapTest, testsasquatch04_0042) { testSingleMap("sasquatch04_0042"); }
TEST_F(SokobanMapTest, testsasquatch04_0043) { testSingleMap("sasquatch04_0043"); }
TEST_F(SokobanMapTest, testsasquatch04_0044) { testSingleMap("sasquatch04_0044"); }
TEST_F(SokobanMapTest, testsasquatch04_0045) { testSingleMap("sasquatch04_0045"); }
TEST_F(SokobanMapTest, testsasquatch04_0046) { testSingleMap("sasquatch04_0046"); }
TEST_F(SokobanMapTest, testsasquatch04_0047) { testSingleMap("sasquatch04_0047"); }
TEST_F(SokobanMapTest, testsasquatch04_0048) { testSingleMap("sasquatch04_0048"); }
TEST_F(SokobanMapTest, testsasquatch04_0049) { testSingleMap("sasquatch04_0049"); }
TEST_F(SokobanMapTest, testsasquatch04_0050) { testSingleMap("sasquatch04_0050"); }
TEST_F(SokobanMapTest, testsasquatch05_0001) { testSingleMap("sasquatch05_0001"); }
TEST_F(SokobanMapTest, testsasquatch05_0002) { testSingleMap("sasquatch05_0002"); }
TEST_F(SokobanMapTest, testsasquatch05_0003) { testSingleMap("sasquatch05_0003"); }
TEST_F(SokobanMapTest, testsasquatch05_0004) { testSingleMap("sasquatch05_0004"); }
TEST_F(SokobanMapTest, testsasquatch05_0005) { testSingleMap("sasquatch05_0005"); }
TEST_F(SokobanMapTest, testsasquatch05_0006) { testSingleMap("sasquatch05_0006"); }
TEST_F(SokobanMapTest, testsasquatch05_0007) { testSingleMap("sasquatch05_0007"); }
TEST_F(SokobanMapTest, testsasquatch05_0008) { testSingleMap("sasquatch05_0008"); }
TEST_F(SokobanMapTest, testsasquatch05_0009) { testSingleMap("sasquatch05_0009"); }
TEST_F(SokobanMapTest, testsasquatch05_0010) { testSingleMap("sasquatch05_0010"); }
TEST_F(SokobanMapTest, testsasquatch05_0011) { testSingleMap("sasquatch05_0011"); }
TEST_F(SokobanMapTest, testsasquatch05_0012) { testSingleMap("sasquatch05_0012"); }
TEST_F(SokobanMapTest, testsasquatch05_0013) { testSingleMap("sasquatch05_0013"); }
TEST_F(SokobanMapTest, testsasquatch05_0014) { testSingleMap("sasquatch05_0014"); }
TEST_F(SokobanMapTest, testsasquatch05_0015) { testSingleMap("sasquatch05_0015"); }
TEST_F(SokobanMapTest, testsasquatch05_0016) { testSingleMap("sasquatch05_0016"); }
TEST_F(SokobanMapTest, testsasquatch05_0017) { testSingleMap("sasquatch05_0017"); }
TEST_F(SokobanMapTest, testsasquatch05_0018) { testSingleMap("sasquatch05_0018"); }
TEST_F(SokobanMapTest, testsasquatch05_0019) { testSingleMap("sasquatch05_0019"); }
TEST_F(SokobanMapTest, testsasquatch05_0020) { testSingleMap("sasquatch05_0020"); }
TEST_F(SokobanMapTest, testsasquatch05_0021) { testSingleMap("sasquatch05_0021"); }
TEST_F(SokobanMapTest, testsasquatch05_0022) { testSingleMap("sasquatch05_0022"); }
TEST_F(SokobanMapTest, testsasquatch05_0023) { testSingleMap("sasquatch05_0023"); }
TEST_F(SokobanMapTest, testsasquatch05_0024) { testSingleMap("sasquatch05_0024"); }
TEST_F(SokobanMapTest, testsasquatch05_0025) { testSingleMap("sasquatch05_0025"); }
TEST_F(SokobanMapTest, testsasquatch05_0026) { testSingleMap("sasquatch05_0026"); }
TEST_F(SokobanMapTest, testsasquatch05_0027) { testSingleMap("sasquatch05_0027"); }
TEST_F(SokobanMapTest, testsasquatch05_0028) { testSingleMap("sasquatch05_0028"); }
TEST_F(SokobanMapTest, testsasquatch05_0029) { testSingleMap("sasquatch05_0029"); }
TEST_F(SokobanMapTest, testsasquatch05_0030) { testSingleMap("sasquatch05_0030"); }
TEST_F(SokobanMapTest, testsasquatch05_0031) { testSingleMap("sasquatch05_0031"); }
TEST_F(SokobanMapTest, testsasquatch05_0032) { testSingleMap("sasquatch05_0032"); }
TEST_F(SokobanMapTest, testsasquatch05_0033) { testSingleMap("sasquatch05_0033"); }
TEST_F(SokobanMapTest, testsasquatch05_0034) { testSingleMap("sasquatch05_0034"); }
TEST_F(SokobanMapTest, testsasquatch05_0035) { testSingleMap("sasquatch05_0035"); }
TEST_F(SokobanMapTest, testsasquatch05_0036) { testSingleMap("sasquatch05_0036"); }
TEST_F(SokobanMapTest, testsasquatch05_0037) { testSingleMap("sasquatch05_0037"); }
TEST_F(SokobanMapTest, testsasquatch05_0038) { testSingleMap("sasquatch05_0038"); }
TEST_F(SokobanMapTest, testsasquatch05_0039) { testSingleMap("sasquatch05_0039"); }
TEST_F(SokobanMapTest, testsasquatch05_0040) { testSingleMap("sasquatch05_0040"); }
TEST_F(SokobanMapTest, testsasquatch05_0041) { testSingleMap("sasquatch05_0041"); }
TEST_F(SokobanMapTest, testsasquatch05_0042) { testSingleMap("sasquatch05_0042"); }
TEST_F(SokobanMapTest, testsasquatch05_0043) { testSingleMap("sasquatch05_0043"); }
TEST_F(SokobanMapTest, testsasquatch05_0044) { testSingleMap("sasquatch05_0044"); }
TEST_F(SokobanMapTest, testsasquatch05_0045) { testSingleMap("sasquatch05_0045"); }
TEST_F(SokobanMapTest, testsasquatch05_0046) { testSingleMap("sasquatch05_0046"); }
TEST_F(SokobanMapTest, testsasquatch05_0047) { testSingleMap("sasquatch05_0047"); }
TEST_F(SokobanMapTest, testsasquatch05_0048) { testSingleMap("sasquatch05_0048"); }
TEST_F(SokobanMapTest, testsasquatch05_0049) { testSingleMap("sasquatch05_0049"); }
TEST_F(SokobanMapTest, testsasquatch05_0050) { testSingleMap("sasquatch05_0050"); }
TEST_F(SokobanMapTest, testsasquatch06_0001) { testSingleMap("sasquatch06_0001"); }
TEST_F(SokobanMapTest, testsasquatch06_0002) { testSingleMap("sasquatch06_0002"); }
TEST_F(SokobanMapTest, testsasquatch06_0003) { testSingleMap("sasquatch06_0003"); }
TEST_F(SokobanMapTest, testsasquatch06_0004) { testSingleMap("sasquatch06_0004"); }
TEST_F(SokobanMapTest, testsasquatch06_0005) { testSingleMap("sasquatch06_0005"); }
TEST_F(SokobanMapTest, testsasquatch06_0006) { testSingleMap("sasquatch06_0006"); }
TEST_F(SokobanMapTest, testsasquatch06_0007) { testSingleMap("sasquatch06_0007"); }
TEST_F(SokobanMapTest, testsasquatch06_0008) { testSingleMap("sasquatch06_0008"); }
TEST_F(SokobanMapTest, testsasquatch06_0009) { testSingleMap("sasquatch06_0009"); }
TEST_F(SokobanMapTest, testsasquatch06_0010) { testSingleMap("sasquatch06_0010"); }
TEST_F(SokobanMapTest, testsasquatch06_0011) { testSingleMap("sasquatch06_0011"); }
TEST_F(SokobanMapTest, testsasquatch06_0012) { testSingleMap("sasquatch06_0012"); }
TEST_F(SokobanMapTest, testsasquatch06_0013) { testSingleMap("sasquatch06_0013"); }
TEST_F(SokobanMapTest, testsasquatch06_0014) { testSingleMap("sasquatch06_0014"); }
TEST_F(SokobanMapTest, testsasquatch06_0015) { testSingleMap("sasquatch06_0015"); }
TEST_F(SokobanMapTest, testsasquatch06_0016) { testSingleMap("sasquatch06_0016"); }
TEST_F(SokobanMapTest, testsasquatch06_0017) { testSingleMap("sasquatch06_0017"); }
TEST_F(SokobanMapTest, testsasquatch06_0018) { testSingleMap("sasquatch06_0018"); }
TEST_F(SokobanMapTest, testsasquatch06_0019) { testSingleMap("sasquatch06_0019"); }
TEST_F(SokobanMapTest, testsasquatch06_0020) { testSingleMap("sasquatch06_0020"); }
TEST_F(SokobanMapTest, testsasquatch06_0021) { testSingleMap("sasquatch06_0021"); }
TEST_F(SokobanMapTest, testsasquatch06_0022) { testSingleMap("sasquatch06_0022"); }
TEST_F(SokobanMapTest, testsasquatch06_0023) { testSingleMap("sasquatch06_0023"); }
TEST_F(SokobanMapTest, testsasquatch06_0024) { testSingleMap("sasquatch06_0024"); }
TEST_F(SokobanMapTest, testsasquatch06_0025) { testSingleMap("sasquatch06_0025"); }
TEST_F(SokobanMapTest, testsasquatch06_0026) { testSingleMap("sasquatch06_0026"); }
TEST_F(SokobanMapTest, testsasquatch06_0027) { testSingleMap("sasquatch06_0027"); }
TEST_F(SokobanMapTest, testsasquatch06_0028) { testSingleMap("sasquatch06_0028"); }
TEST_F(SokobanMapTest, testsasquatch06_0029) { testSingleMap("sasquatch06_0029"); }
TEST_F(SokobanMapTest, testsasquatch06_0030) { testSingleMap("sasquatch06_0030"); }
TEST_F(SokobanMapTest, testsasquatch06_0031) { testSingleMap("sasquatch06_0031"); }
TEST_F(SokobanMapTest, testsasquatch06_0032) { testSingleMap("sasquatch06_0032"); }
TEST_F(SokobanMapTest, testsasquatch06_0033) { testSingleMap("sasquatch06_0033"); }
TEST_F(SokobanMapTest, testsasquatch06_0034) { testSingleMap("sasquatch06_0034"); }
TEST_F(SokobanMapTest, testsasquatch06_0035) { testSingleMap("sasquatch06_0035"); }
TEST_F(SokobanMapTest, testsasquatch06_0036) { testSingleMap("sasquatch06_0036"); }
TEST_F(SokobanMapTest, testsasquatch06_0037) { testSingleMap("sasquatch06_0037"); }
TEST_F(SokobanMapTest, testsasquatch06_0038) { testSingleMap("sasquatch06_0038"); }
TEST_F(SokobanMapTest, testsasquatch06_0039) { testSingleMap("sasquatch06_0039"); }
TEST_F(SokobanMapTest, testsasquatch06_0040) { testSingleMap("sasquatch06_0040"); }
TEST_F(SokobanMapTest, testsasquatch06_0041) { testSingleMap("sasquatch06_0041"); }
TEST_F(SokobanMapTest, testsasquatch06_0042) { testSingleMap("sasquatch06_0042"); }
TEST_F(SokobanMapTest, testsasquatch06_0043) { testSingleMap("sasquatch06_0043"); }
TEST_F(SokobanMapTest, testsasquatch06_0044) { testSingleMap("sasquatch06_0044"); }
TEST_F(SokobanMapTest, testsasquatch06_0045) { testSingleMap("sasquatch06_0045"); }
TEST_F(SokobanMapTest, testsasquatch06_0046) { testSingleMap("sasquatch06_0046"); }
TEST_F(SokobanMapTest, testsasquatch06_0047) { testSingleMap("sasquatch06_0047"); }
TEST_F(SokobanMapTest, testsasquatch06_0048) { testSingleMap("sasquatch06_0048"); }
TEST_F(SokobanMapTest, testsasquatch06_0049) { testSingleMap("sasquatch06_0049"); }
TEST_F(SokobanMapTest, testsasquatch06_0050) { testSingleMap("sasquatch06_0050"); }
TEST_F(SokobanMapTest, testsasquatch07_0001) { testSingleMap("sasquatch07_0001"); }
TEST_F(SokobanMapTest, testsasquatch07_0002) { testSingleMap("sasquatch07_0002"); }
TEST_F(SokobanMapTest, testsasquatch07_0003) { testSingleMap("sasquatch07_0003"); }
TEST_F(SokobanMapTest, testsasquatch07_0004) { testSingleMap("sasquatch07_0004"); }
TEST_F(SokobanMapTest, testsasquatch07_0005) { testSingleMap("sasquatch07_0005"); }
TEST_F(SokobanMapTest, testsasquatch07_0006) { testSingleMap("sasquatch07_0006"); }
TEST_F(SokobanMapTest, testsasquatch07_0007) { testSingleMap("sasquatch07_0007"); }
TEST_F(SokobanMapTest, testsasquatch07_0008) { testSingleMap("sasquatch07_0008"); }
TEST_F(SokobanMapTest, testsasquatch07_0009) { testSingleMap("sasquatch07_0009"); }
TEST_F(SokobanMapTest, testsasquatch07_0010) { testSingleMap("sasquatch07_0010"); }
TEST_F(SokobanMapTest, testsasquatch07_0011) { testSingleMap("sasquatch07_0011"); }
TEST_F(SokobanMapTest, testsasquatch07_0012) { testSingleMap("sasquatch07_0012"); }
TEST_F(SokobanMapTest, testsasquatch07_0013) { testSingleMap("sasquatch07_0013"); }
TEST_F(SokobanMapTest, testsasquatch07_0014) { testSingleMap("sasquatch07_0014"); }
TEST_F(SokobanMapTest, testsasquatch07_0015) { testSingleMap("sasquatch07_0015"); }
TEST_F(SokobanMapTest, testsasquatch07_0016) { testSingleMap("sasquatch07_0016"); }
TEST_F(SokobanMapTest, testsasquatch07_0017) { testSingleMap("sasquatch07_0017"); }
TEST_F(SokobanMapTest, testsasquatch07_0018) { testSingleMap("sasquatch07_0018"); }
TEST_F(SokobanMapTest, testsasquatch07_0019) { testSingleMap("sasquatch07_0019"); }
TEST_F(SokobanMapTest, testsasquatch07_0020) { testSingleMap("sasquatch07_0020"); }
TEST_F(SokobanMapTest, testsasquatch07_0021) { testSingleMap("sasquatch07_0021"); }
TEST_F(SokobanMapTest, testsasquatch07_0022) { testSingleMap("sasquatch07_0022"); }
TEST_F(SokobanMapTest, testsasquatch07_0023) { testSingleMap("sasquatch07_0023"); }
TEST_F(SokobanMapTest, testsasquatch07_0024) { testSingleMap("sasquatch07_0024"); }
TEST_F(SokobanMapTest, testsasquatch07_0025) { testSingleMap("sasquatch07_0025"); }
TEST_F(SokobanMapTest, testsasquatch07_0026) { testSingleMap("sasquatch07_0026"); }
TEST_F(SokobanMapTest, testsasquatch07_0027) { testSingleMap("sasquatch07_0027"); }
TEST_F(SokobanMapTest, testsasquatch07_0028) { testSingleMap("sasquatch07_0028"); }
TEST_F(SokobanMapTest, testsasquatch07_0029) { testSingleMap("sasquatch07_0029"); }
TEST_F(SokobanMapTest, testsasquatch07_0030) { testSingleMap("sasquatch07_0030"); }
TEST_F(SokobanMapTest, testsasquatch07_0031) { testSingleMap("sasquatch07_0031"); }
TEST_F(SokobanMapTest, testsasquatch07_0032) { testSingleMap("sasquatch07_0032"); }
TEST_F(SokobanMapTest, testsasquatch07_0033) { testSingleMap("sasquatch07_0033"); }
TEST_F(SokobanMapTest, testsasquatch07_0034) { testSingleMap("sasquatch07_0034"); }
TEST_F(SokobanMapTest, testsasquatch07_0035) { testSingleMap("sasquatch07_0035"); }
TEST_F(SokobanMapTest, testsasquatch07_0036) { testSingleMap("sasquatch07_0036"); }
TEST_F(SokobanMapTest, testsasquatch07_0037) { testSingleMap("sasquatch07_0037"); }
TEST_F(SokobanMapTest, testsasquatch07_0038) { testSingleMap("sasquatch07_0038"); }
TEST_F(SokobanMapTest, testsasquatch07_0039) { testSingleMap("sasquatch07_0039"); }
TEST_F(SokobanMapTest, testsasquatch07_0040) { testSingleMap("sasquatch07_0040"); }
TEST_F(SokobanMapTest, testsasquatch07_0041) { testSingleMap("sasquatch07_0041"); }
TEST_F(SokobanMapTest, testsasquatch07_0042) { testSingleMap("sasquatch07_0042"); }
TEST_F(SokobanMapTest, testsasquatch07_0043) { testSingleMap("sasquatch07_0043"); }
TEST_F(SokobanMapTest, testsasquatch07_0044) { testSingleMap("sasquatch07_0044"); }
TEST_F(SokobanMapTest, testsasquatch07_0045) { testSingleMap("sasquatch07_0045"); }
TEST_F(SokobanMapTest, testsasquatch07_0046) { testSingleMap("sasquatch07_0046"); }
TEST_F(SokobanMapTest, testsasquatch07_0047) { testSingleMap("sasquatch07_0047"); }
TEST_F(SokobanMapTest, testsasquatch07_0048) { testSingleMap("sasquatch07_0048"); }
TEST_F(SokobanMapTest, testsasquatch07_0049) { testSingleMap("sasquatch07_0049"); }
TEST_F(SokobanMapTest, testsasquatch07_0050) { testSingleMap("sasquatch07_0050"); }
TEST_F(SokobanMapTest, testsasquatch08_0001) { testSingleMap("sasquatch08_0001"); }
TEST_F(SokobanMapTest, testsasquatch08_0002) { testSingleMap("sasquatch08_0002"); }
TEST_F(SokobanMapTest, testsasquatch08_0003) { testSingleMap("sasquatch08_0003"); }
TEST_F(SokobanMapTest, testsasquatch08_0004) { testSingleMap("sasquatch08_0004"); }
TEST_F(SokobanMapTest, testsasquatch08_0005) { testSingleMap("sasquatch08_0005"); }
TEST_F(SokobanMapTest, testsasquatch08_0006) { testSingleMap("sasquatch08_0006"); }
TEST_F(SokobanMapTest, testsasquatch08_0007) { testSingleMap("sasquatch08_0007"); }
TEST_F(SokobanMapTest, testsasquatch08_0008) { testSingleMap("sasquatch08_0008"); }
TEST_F(SokobanMapTest, testsasquatch08_0009) { testSingleMap("sasquatch08_0009"); }
TEST_F(SokobanMapTest, testsasquatch08_0010) { testSingleMap("sasquatch08_0010"); }
TEST_F(SokobanMapTest, testsasquatch08_0011) { testSingleMap("sasquatch08_0011"); }
TEST_F(SokobanMapTest, testsasquatch08_0012) { testSingleMap("sasquatch08_0012"); }
TEST_F(SokobanMapTest, testsasquatch08_0013) { testSingleMap("sasquatch08_0013"); }
TEST_F(SokobanMapTest, testsasquatch08_0014) { testSingleMap("sasquatch08_0014"); }
TEST_F(SokobanMapTest, testsasquatch08_0015) { testSingleMap("sasquatch08_0015"); }
TEST_F(SokobanMapTest, testsasquatch08_0016) { testSingleMap("sasquatch08_0016"); }
TEST_F(SokobanMapTest, testsasquatch08_0017) { testSingleMap("sasquatch08_0017"); }
TEST_F(SokobanMapTest, testsasquatch08_0018) { testSingleMap("sasquatch08_0018"); }
TEST_F(SokobanMapTest, testsasquatch08_0019) { testSingleMap("sasquatch08_0019"); }
TEST_F(SokobanMapTest, testsasquatch08_0020) { testSingleMap("sasquatch08_0020"); }
TEST_F(SokobanMapTest, testsasquatch08_0021) { testSingleMap("sasquatch08_0021"); }
TEST_F(SokobanMapTest, testsasquatch08_0022) { testSingleMap("sasquatch08_0022"); }
TEST_F(SokobanMapTest, testsasquatch08_0023) { testSingleMap("sasquatch08_0023"); }
TEST_F(SokobanMapTest, testsasquatch08_0024) { testSingleMap("sasquatch08_0024"); }
TEST_F(SokobanMapTest, testsasquatch08_0025) { testSingleMap("sasquatch08_0025"); }
TEST_F(SokobanMapTest, testsasquatch08_0026) { testSingleMap("sasquatch08_0026"); }
TEST_F(SokobanMapTest, testsasquatch08_0027) { testSingleMap("sasquatch08_0027"); }
TEST_F(SokobanMapTest, testsasquatch08_0028) { testSingleMap("sasquatch08_0028"); }
TEST_F(SokobanMapTest, testsasquatch08_0029) { testSingleMap("sasquatch08_0029"); }
TEST_F(SokobanMapTest, testsasquatch08_0030) { testSingleMap("sasquatch08_0030"); }
TEST_F(SokobanMapTest, testsasquatch08_0031) { testSingleMap("sasquatch08_0031"); }
TEST_F(SokobanMapTest, testsasquatch08_0032) { testSingleMap("sasquatch08_0032"); }
TEST_F(SokobanMapTest, testsasquatch08_0033) { testSingleMap("sasquatch08_0033"); }
TEST_F(SokobanMapTest, testsasquatch08_0034) { testSingleMap("sasquatch08_0034"); }
TEST_F(SokobanMapTest, testsasquatch08_0035) { testSingleMap("sasquatch08_0035"); }
TEST_F(SokobanMapTest, testsasquatch08_0036) { testSingleMap("sasquatch08_0036"); }
TEST_F(SokobanMapTest, testsasquatch08_0037) { testSingleMap("sasquatch08_0037"); }
TEST_F(SokobanMapTest, testsasquatch08_0038) { testSingleMap("sasquatch08_0038"); }
TEST_F(SokobanMapTest, testsasquatch08_0039) { testSingleMap("sasquatch08_0039"); }
TEST_F(SokobanMapTest, testsasquatch08_0040) { testSingleMap("sasquatch08_0040"); }
TEST_F(SokobanMapTest, testsasquatch08_0041) { testSingleMap("sasquatch08_0041"); }
TEST_F(SokobanMapTest, testsasquatch08_0042) { testSingleMap("sasquatch08_0042"); }
TEST_F(SokobanMapTest, testsasquatch08_0043) { testSingleMap("sasquatch08_0043"); }
TEST_F(SokobanMapTest, testsasquatch08_0044) { testSingleMap("sasquatch08_0044"); }
TEST_F(SokobanMapTest, testsasquatch08_0045) { testSingleMap("sasquatch08_0045"); }
TEST_F(SokobanMapTest, testsasquatch08_0046) { testSingleMap("sasquatch08_0046"); }
TEST_F(SokobanMapTest, testsasquatch08_0047) { testSingleMap("sasquatch08_0047"); }
TEST_F(SokobanMapTest, testsasquatch08_0048) { testSingleMap("sasquatch08_0048"); }
TEST_F(SokobanMapTest, testsasquatch08_0049) { testSingleMap("sasquatch08_0049"); }
TEST_F(SokobanMapTest, testsasquatch08_0050) { testSingleMap("sasquatch08_0050"); }
TEST_F(SokobanMapTest, testsasquatch09_0001) { testSingleMap("sasquatch09_0001"); }
TEST_F(SokobanMapTest, testsasquatch09_0002) { testSingleMap("sasquatch09_0002"); }
TEST_F(SokobanMapTest, testsasquatch09_0003) { testSingleMap("sasquatch09_0003"); }
TEST_F(SokobanMapTest, testsasquatch09_0004) { testSingleMap("sasquatch09_0004"); }
TEST_F(SokobanMapTest, testsasquatch09_0005) { testSingleMap("sasquatch09_0005"); }
TEST_F(SokobanMapTest, testsasquatch09_0006) { testSingleMap("sasquatch09_0006"); }
TEST_F(SokobanMapTest, testsasquatch09_0007) { testSingleMap("sasquatch09_0007"); }
TEST_F(SokobanMapTest, testsasquatch09_0008) { testSingleMap("sasquatch09_0008"); }
TEST_F(SokobanMapTest, testsasquatch09_0009) { testSingleMap("sasquatch09_0009"); }
TEST_F(SokobanMapTest, testsasquatch09_0010) { testSingleMap("sasquatch09_0010"); }
TEST_F(SokobanMapTest, testsasquatch09_0011) { testSingleMap("sasquatch09_0011"); }
TEST_F(SokobanMapTest, testsasquatch09_0012) { testSingleMap("sasquatch09_0012"); }
TEST_F(SokobanMapTest, testsasquatch09_0013) { testSingleMap("sasquatch09_0013"); }
TEST_F(SokobanMapTest, testsasquatch09_0014) { testSingleMap("sasquatch09_0014"); }
TEST_F(SokobanMapTest, testsasquatch09_0015) { testSingleMap("sasquatch09_0015"); }
TEST_F(SokobanMapTest, testsasquatch09_0016) { testSingleMap("sasquatch09_0016"); }
TEST_F(SokobanMapTest, testsasquatch09_0017) { testSingleMap("sasquatch09_0017"); }
TEST_F(SokobanMapTest, testsasquatch09_0018) { testSingleMap("sasquatch09_0018"); }
TEST_F(SokobanMapTest, testsasquatch09_0019) { testSingleMap("sasquatch09_0019"); }
TEST_F(SokobanMapTest, testsasquatch09_0020) { testSingleMap("sasquatch09_0020"); }
TEST_F(SokobanMapTest, testsasquatch09_0021) { testSingleMap("sasquatch09_0021"); }
TEST_F(SokobanMapTest, testsasquatch09_0022) { testSingleMap("sasquatch09_0022"); }
TEST_F(SokobanMapTest, testsasquatch09_0023) { testSingleMap("sasquatch09_0023"); }
TEST_F(SokobanMapTest, testsasquatch09_0024) { testSingleMap("sasquatch09_0024"); }
TEST_F(SokobanMapTest, testsasquatch09_0025) { testSingleMap("sasquatch09_0025"); }
TEST_F(SokobanMapTest, testsasquatch09_0026) { testSingleMap("sasquatch09_0026"); }
TEST_F(SokobanMapTest, testsasquatch09_0027) { testSingleMap("sasquatch09_0027"); }
TEST_F(SokobanMapTest, testsasquatch09_0028) { testSingleMap("sasquatch09_0028"); }
TEST_F(SokobanMapTest, testsasquatch09_0029) { testSingleMap("sasquatch09_0029"); }
TEST_F(SokobanMapTest, testsasquatch09_0030) { testSingleMap("sasquatch09_0030"); }
TEST_F(SokobanMapTest, testsasquatch09_0031) { testSingleMap("sasquatch09_0031"); }
TEST_F(SokobanMapTest, testsasquatch09_0032) { testSingleMap("sasquatch09_0032"); }
TEST_F(SokobanMapTest, testsasquatch09_0033) { testSingleMap("sasquatch09_0033"); }
TEST_F(SokobanMapTest, testsasquatch09_0034) { testSingleMap("sasquatch09_0034"); }
TEST_F(SokobanMapTest, testsasquatch09_0035) { testSingleMap("sasquatch09_0035"); }
TEST_F(SokobanMapTest, testsasquatch09_0036) { testSingleMap("sasquatch09_0036"); }
TEST_F(SokobanMapTest, testsasquatch09_0037) { testSingleMap("sasquatch09_0037"); }
TEST_F(SokobanMapTest, testsasquatch09_0038) { testSingleMap("sasquatch09_0038"); }
TEST_F(SokobanMapTest, testsasquatch09_0039) { testSingleMap("sasquatch09_0039"); }
TEST_F(SokobanMapTest, testsasquatch09_0040) { testSingleMap("sasquatch09_0040"); }
TEST_F(SokobanMapTest, testsasquatch09_0041) { testSingleMap("sasquatch09_0041"); }
TEST_F(SokobanMapTest, testsasquatch09_0042) { testSingleMap("sasquatch09_0042"); }
TEST_F(SokobanMapTest, testsasquatch09_0043) { testSingleMap("sasquatch09_0043"); }
TEST_F(SokobanMapTest, testsasquatch09_0044) { testSingleMap("sasquatch09_0044"); }
TEST_F(SokobanMapTest, testsasquatch09_0045) { testSingleMap("sasquatch09_0045"); }
TEST_F(SokobanMapTest, testsasquatch09_0046) { testSingleMap("sasquatch09_0046"); }
TEST_F(SokobanMapTest, testsasquatch09_0047) { testSingleMap("sasquatch09_0047"); }
TEST_F(SokobanMapTest, testsasquatch09_0048) { testSingleMap("sasquatch09_0048"); }
TEST_F(SokobanMapTest, testsasquatch09_0049) { testSingleMap("sasquatch09_0049"); }
TEST_F(SokobanMapTest, testsasquatch09_0050) { testSingleMap("sasquatch09_0050"); }
TEST_F(SokobanMapTest, testsasquatch10_0001) { testSingleMap("sasquatch10_0001"); }
TEST_F(SokobanMapTest, testsasquatch10_0002) { testSingleMap("sasquatch10_0002"); }
TEST_F(SokobanMapTest, testsasquatch10_0003) { testSingleMap("sasquatch10_0003"); }
TEST_F(SokobanMapTest, testsasquatch10_0004) { testSingleMap("sasquatch10_0004"); }
TEST_F(SokobanMapTest, testsasquatch10_0005) { testSingleMap("sasquatch10_0005"); }
TEST_F(SokobanMapTest, testsasquatch10_0006) { testSingleMap("sasquatch10_0006"); }
TEST_F(SokobanMapTest, testsasquatch10_0007) { testSingleMap("sasquatch10_0007"); }
TEST_F(SokobanMapTest, testsasquatch10_0008) { testSingleMap("sasquatch10_0008"); }
TEST_F(SokobanMapTest, testsasquatch10_0009) { testSingleMap("sasquatch10_0009"); }
TEST_F(SokobanMapTest, testsasquatch10_0010) { testSingleMap("sasquatch10_0010"); }
TEST_F(SokobanMapTest, testsasquatch10_0011) { testSingleMap("sasquatch10_0011"); }
TEST_F(SokobanMapTest, testsasquatch10_0012) { testSingleMap("sasquatch10_0012"); }
TEST_F(SokobanMapTest, testsasquatch10_0013) { testSingleMap("sasquatch10_0013"); }
TEST_F(SokobanMapTest, testsasquatch10_0014) { testSingleMap("sasquatch10_0014"); }
TEST_F(SokobanMapTest, testsasquatch10_0015) { testSingleMap("sasquatch10_0015"); }
TEST_F(SokobanMapTest, testsasquatch10_0016) { testSingleMap("sasquatch10_0016"); }
TEST_F(SokobanMapTest, testsasquatch10_0017) { testSingleMap("sasquatch10_0017"); }
TEST_F(SokobanMapTest, testsasquatch10_0018) { testSingleMap("sasquatch10_0018"); }
TEST_F(SokobanMapTest, testsasquatch10_0019) { testSingleMap("sasquatch10_0019"); }
TEST_F(SokobanMapTest, testsasquatch10_0020) { testSingleMap("sasquatch10_0020"); }
TEST_F(SokobanMapTest, testsasquatch10_0021) { testSingleMap("sasquatch10_0021"); }
TEST_F(SokobanMapTest, testsasquatch10_0022) { testSingleMap("sasquatch10_0022"); }
TEST_F(SokobanMapTest, testsasquatch10_0023) { testSingleMap("sasquatch10_0023"); }
TEST_F(SokobanMapTest, testsasquatch10_0024) { testSingleMap("sasquatch10_0024"); }
TEST_F(SokobanMapTest, testsasquatch10_0025) { testSingleMap("sasquatch10_0025"); }
TEST_F(SokobanMapTest, testsasquatch10_0026) { testSingleMap("sasquatch10_0026"); }
TEST_F(SokobanMapTest, testsasquatch10_0027) { testSingleMap("sasquatch10_0027"); }
TEST_F(SokobanMapTest, testsasquatch10_0028) { testSingleMap("sasquatch10_0028"); }
TEST_F(SokobanMapTest, testsasquatch10_0029) { testSingleMap("sasquatch10_0029"); }
TEST_F(SokobanMapTest, testsasquatch10_0030) { testSingleMap("sasquatch10_0030"); }
TEST_F(SokobanMapTest, testsasquatch10_0031) { testSingleMap("sasquatch10_0031"); }
TEST_F(SokobanMapTest, testsasquatch10_0032) { testSingleMap("sasquatch10_0032"); }
TEST_F(SokobanMapTest, testsasquatch10_0033) { testSingleMap("sasquatch10_0033"); }
TEST_F(SokobanMapTest, testsasquatch10_0034) { testSingleMap("sasquatch10_0034"); }
TEST_F(SokobanMapTest, testsasquatch10_0035) { testSingleMap("sasquatch10_0035"); }
TEST_F(SokobanMapTest, testsasquatch10_0036) { testSingleMap("sasquatch10_0036"); }
TEST_F(SokobanMapTest, testsasquatch10_0037) { testSingleMap("sasquatch10_0037"); }
TEST_F(SokobanMapTest, testsasquatch10_0038) { testSingleMap("sasquatch10_0038"); }
TEST_F(SokobanMapTest, testsasquatch10_0039) { testSingleMap("sasquatch10_0039"); }
TEST_F(SokobanMapTest, testsasquatch10_0040) { testSingleMap("sasquatch10_0040"); }
TEST_F(SokobanMapTest, testsasquatch10_0041) { testSingleMap("sasquatch10_0041"); }
TEST_F(SokobanMapTest, testsasquatch10_0042) { testSingleMap("sasquatch10_0042"); }
TEST_F(SokobanMapTest, testsasquatch10_0043) { testSingleMap("sasquatch10_0043"); }
TEST_F(SokobanMapTest, testsasquatch10_0044) { testSingleMap("sasquatch10_0044"); }
TEST_F(SokobanMapTest, testsasquatch10_0045) { testSingleMap("sasquatch10_0045"); }
TEST_F(SokobanMapTest, testsasquatch10_0046) { testSingleMap("sasquatch10_0046"); }
TEST_F(SokobanMapTest, testsasquatch10_0047) { testSingleMap("sasquatch10_0047"); }
TEST_F(SokobanMapTest, testsasquatch10_0048) { testSingleMap("sasquatch10_0048"); }
TEST_F(SokobanMapTest, testsasquatch10_0049) { testSingleMap("sasquatch10_0049"); }
TEST_F(SokobanMapTest, testsasquatch10_0050) { testSingleMap("sasquatch10_0050"); }
TEST_F(SokobanMapTest, testsasquatch11_0001) { testSingleMap("sasquatch11_0001"); }
TEST_F(SokobanMapTest, testsasquatch11_0002) { testSingleMap("sasquatch11_0002"); }
TEST_F(SokobanMapTest, testsasquatch11_0003) { testSingleMap("sasquatch11_0003"); }
TEST_F(SokobanMapTest, testsasquatch11_0004) { testSingleMap("sasquatch11_0004"); }
TEST_F(SokobanMapTest, testsasquatch11_0005) { testSingleMap("sasquatch11_0005"); }
TEST_F(SokobanMapTest, testsasquatch11_0006) { testSingleMap("sasquatch11_0006"); }
TEST_F(SokobanMapTest, testsasquatch11_0007) { testSingleMap("sasquatch11_0007"); }
TEST_F(SokobanMapTest, testsasquatch11_0008) { testSingleMap("sasquatch11_0008"); }
TEST_F(SokobanMapTest, testsasquatch11_0009) { testSingleMap("sasquatch11_0009"); }
TEST_F(SokobanMapTest, testsasquatch11_0010) { testSingleMap("sasquatch11_0010"); }
TEST_F(SokobanMapTest, testsasquatch11_0011) { testSingleMap("sasquatch11_0011"); }
TEST_F(SokobanMapTest, testsasquatch11_0012) { testSingleMap("sasquatch11_0012"); }
TEST_F(SokobanMapTest, testsasquatch11_0013) { testSingleMap("sasquatch11_0013"); }
TEST_F(SokobanMapTest, testsasquatch11_0014) { testSingleMap("sasquatch11_0014"); }
TEST_F(SokobanMapTest, testsasquatch11_0015) { testSingleMap("sasquatch11_0015"); }
TEST_F(SokobanMapTest, testsasquatch11_0016) { testSingleMap("sasquatch11_0016"); }
TEST_F(SokobanMapTest, testsasquatch11_0017) { testSingleMap("sasquatch11_0017"); }
TEST_F(SokobanMapTest, testsasquatch11_0018) { testSingleMap("sasquatch11_0018"); }
TEST_F(SokobanMapTest, testsasquatch11_0019) { testSingleMap("sasquatch11_0019"); }
TEST_F(SokobanMapTest, testsasquatch11_0020) { testSingleMap("sasquatch11_0020"); }
TEST_F(SokobanMapTest, testsasquatch11_0021) { testSingleMap("sasquatch11_0021"); }
TEST_F(SokobanMapTest, testsasquatch11_0022) { testSingleMap("sasquatch11_0022"); }
TEST_F(SokobanMapTest, testsasquatch11_0023) { testSingleMap("sasquatch11_0023"); }
TEST_F(SokobanMapTest, testsasquatch11_0024) { testSingleMap("sasquatch11_0024"); }
TEST_F(SokobanMapTest, testsasquatch11_0025) { testSingleMap("sasquatch11_0025"); }
TEST_F(SokobanMapTest, testsasquatch11_0026) { testSingleMap("sasquatch11_0026"); }
TEST_F(SokobanMapTest, testsasquatch11_0027) { testSingleMap("sasquatch11_0027"); }
TEST_F(SokobanMapTest, testsasquatch11_0028) { testSingleMap("sasquatch11_0028"); }
TEST_F(SokobanMapTest, testsasquatch11_0029) { testSingleMap("sasquatch11_0029"); }
TEST_F(SokobanMapTest, testsasquatch11_0030) { testSingleMap("sasquatch11_0030"); }
TEST_F(SokobanMapTest, testsasquatch11_0031) { testSingleMap("sasquatch11_0031"); }
TEST_F(SokobanMapTest, testsasquatch11_0032) { testSingleMap("sasquatch11_0032"); }
TEST_F(SokobanMapTest, testsasquatch11_0033) { testSingleMap("sasquatch11_0033"); }
TEST_F(SokobanMapTest, testsasquatch11_0034) { testSingleMap("sasquatch11_0034"); }
TEST_F(SokobanMapTest, testsasquatch11_0035) { testSingleMap("sasquatch11_0035"); }
TEST_F(SokobanMapTest, testsasquatch11_0036) { testSingleMap("sasquatch11_0036"); }
TEST_F(SokobanMapTest, testsasquatch11_0037) { testSingleMap("sasquatch11_0037"); }
TEST_F(SokobanMapTest, testsasquatch11_0038) { testSingleMap("sasquatch11_0038"); }
TEST_F(SokobanMapTest, testsasquatch11_0039) { testSingleMap("sasquatch11_0039"); }
TEST_F(SokobanMapTest, testsasquatch11_0040) { testSingleMap("sasquatch11_0040"); }
TEST_F(SokobanMapTest, testsasquatch11_0041) { testSingleMap("sasquatch11_0041"); }
TEST_F(SokobanMapTest, testsasquatch11_0042) { testSingleMap("sasquatch11_0042"); }
TEST_F(SokobanMapTest, testsasquatch11_0043) { testSingleMap("sasquatch11_0043"); }
TEST_F(SokobanMapTest, testsasquatch11_0044) { testSingleMap("sasquatch11_0044"); }
TEST_F(SokobanMapTest, testsasquatch11_0045) { testSingleMap("sasquatch11_0045"); }
TEST_F(SokobanMapTest, testsasquatch11_0046) { testSingleMap("sasquatch11_0046"); }
TEST_F(SokobanMapTest, testsasquatch11_0047) { testSingleMap("sasquatch11_0047"); }
TEST_F(SokobanMapTest, testsasquatch11_0048) { testSingleMap("sasquatch11_0048"); }
TEST_F(SokobanMapTest, testsasquatch11_0049) { testSingleMap("sasquatch11_0049"); }
TEST_F(SokobanMapTest, testsasquatch11_0050) { testSingleMap("sasquatch11_0050"); }
