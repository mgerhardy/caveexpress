#include "tests/TestShared.h"
#include "common/Common.h"
#include "cavepacker/shared/Pathfinding.cpp"
#include "cavepacker/shared/BoardStateUtil.h"

namespace cavepacker {

#define SCOPE(mapStr) BoardState s; fillState(s, (mapStr), false); SCOPED_TRACE(va("%s\n%s", __PRETTY_FUNCTION__, s.toString().c_str()))

class PathfindingTest: public AbstractTest {
protected:
	inline void fillState(BoardState& s, const char* board, bool convertPlayers = true) const {
		ASSERT_TRUE(createBoardStateFromString(s, board, convertPlayers)) << "could not fill the board state with " << board;
	}
};

TEST_F(PathfindingTest, testOpenList) {
	NodeList list(3);
	ASSERT_TRUE(list.empty());

	Node node1;
	node1.index = 1;
	node1.f = 3;
	list.insert(node1);
	ASSERT_FALSE(list.empty());

	Node node2;
	node2.index = 2;
	node2.f = 1;
	list.insert(node2);
	ASSERT_FALSE(list.empty());

	Node node3;
	node3.index = 2;
	node3.f = 10;
	list.insert(node3);
	ASSERT_FALSE(list.empty());

	ASSERT_NE(list.end(), list.find(node1));
	ASSERT_EQ(node1, *list.find(node1));

	ASSERT_NE(list.end(), list.find(node2));
	ASSERT_EQ(node2, *list.find(node2));

	list.close(*list.find(node2));
	ASSERT_TRUE(list.isClosed(node2));
	ASSERT_TRUE(list.isClosed(*list.find(node2)));

	ASSERT_EQ(node2, list.pop());
	ASSERT_EQ(node1, list.pop());
	ASSERT_EQ(node3, list.pop());

	ASSERT_TRUE(list.empty());

	ASSERT_EQ(list.end(), list.find(node1));
	ASSERT_EQ(list.end(), list.find(node2));
	ASSERT_EQ(list.end(), list.find(node3));
}

TEST_F(PathfindingTest, testSimpleRoute) {
	const char* mapStr =
		"############\n"
		"#$$  #     ###\n"
		"#$$  #       #\n"
		"#$$  # ####  #\n"
		"#$$      ##  #\n"
		"#$$  # #    ##\n"
		"###### ##    #\n"
		"#          #\n"
		"#    #     #\n"
		"############";

	SCOPE(mapStr);
	ASSERT_EQ(14, s.getWidth());
	ASSERT_EQ(10, s.getHeight());
#if 0
	ASSERT_TRUE(s.clearField(4, 3));
	ASSERT_TRUE(s.clearField(6, 3));
	ASSERT_TRUE(s.setField(4, 3, Sokoban::VISITED));
	ASSERT_TRUE(s.setField(6, 3, Sokoban::VISITED));
#endif
	std::vector<int> path;
	ASSERT_TRUE(astar(s, s.getIndex(4, 3), s.getIndex(6, 1), path)) << "Could not find the route from 5, 4 to 7, 2";
	ASSERT_EQ(7u, path.size());
	ASSERT_EQ(s.getIndex(4, 3), path[0]);
	ASSERT_EQ(s.getIndex(4, 4), path[1]);
	ASSERT_EQ(s.getIndex(5, 4), path[2]);
	ASSERT_EQ(s.getIndex(6, 4), path[3]);
	ASSERT_EQ(s.getIndex(6, 3), path[4]);
	ASSERT_EQ(s.getIndex(6, 2), path[5]);
	ASSERT_EQ(s.getIndex(6, 1), path[6]);
}

}
