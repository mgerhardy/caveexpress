#include "tests/TestShared.h"
#include "common/Common.h"
#include "cavepacker/server/map/BoardState.h"
#include "cavepacker/server/map/BoardStateUtil.h"
#include "cavepacker/server/map/deadlock/DeadlockDetector.h"

namespace cavepacker {

#define SCOPE(mapStr) BoardState s; fillState(s, (mapStr)); SCOPED_TRACE(va("%s\n%s", __PRETTY_FUNCTION__, s.toString().c_str()))

class BoardStateTest: public AbstractTest {
protected:
	inline void fillState(BoardState& s, const char* board, bool convertPlayers = true) const {
		ASSERT_TRUE(createBoardStateFromString(s, board, convertPlayers)) << "could not fill the board state with " << board;
	}

	template<typename T>
	std::string vecToString(const std::vector<T>& vec) const {
		std::string str;
		for (auto & v : vec) {
			if (!str.empty())
				str.append(", ");
			str.append(std::to_string(v));
		}
		return str;
	}

	std::string indicesToColRowString(const BoardState& s, const std::vector<int>& vec) const {
		std::string str;
		for (int index : vec) {
			if (!str.empty())
				str.append(", ");
			str.append("[");
			int col;
			int row;
			if (!s.getColRowFromIndex(index, col, row)) {
				str.append("invalid");
			} else {
				str.append(std::to_string(col));
				str.append(", ");
				str.append(std::to_string(row));
			}
			str.append("]");
		}
		return str;
	}

	void testDeadlock(const char *mapStr) {
		SCOPE(mapStr);
		ASSERT_TRUE(s.hasDeadlock());
	}

	void testNoSimpleDeadlock(const char *mapStr) {
		SCOPE(mapStr);
		SimpleDeadlockDetector simple;
		simple.init(s);
		ASSERT_FALSE(simple.hasDeadlock(s)) << "Blocked fields: " << getDeadlocks(simple, s);
	}

	void testNoSimpleDeadlockAt(const char *mapStr, int col, int row) {
		SCOPE(mapStr);
		SimpleDeadlockDetector simple;
		simple.init(s);
		const int index = s.getIndex(col, row);
		const bool deadlock = simple.hasDeadlockAt(index);
		ASSERT_FALSE(deadlock) << "Unexpected deadlock found at " << (col + 1) << ":" << (row + 1) << " index: " << index << ". Blocked fields: " << getDeadlocks(simple, s);
	}

	void testSimpleDeadlockAt(const char *mapStr, int col, int row) {
		SCOPE(mapStr);
		SimpleDeadlockDetector simple;
		simple.init(s);
		ASSERT_TRUE(simple.hasDeadlockAt(s.getIndex(col, row))) << "Expected deadlock not found at " << (col + 1) << ":" << (row + 1) << ". Blocked fields: " << getDeadlocks(simple, s);
	}

	void testSimpleDeadlock(const char *mapStr) {
		SCOPE(mapStr);
		SimpleDeadlockDetector simple;
		simple.init(s);
		ASSERT_TRUE(simple.hasDeadlock(s)) << "Blocked fields: " << getDeadlocks(simple, s);
	}

	void testNoDeadlock(const char *mapStr) {
		SCOPE(mapStr);
		ASSERT_FALSE(s.hasDeadlock());
	}

	void testFrozenDeadlock(const char *mapStr) {
		SCOPE(mapStr);
		SimpleDeadlockDetector simple;
		simple.init(s);
		FrozenDeadlockDetector frozen;
		frozen.init(s);
		ASSERT_TRUE(frozen.hasDeadlock(simple, s)) << "Blocked fields: " << getDeadlocks(frozen, s);
	}

	void testNoFrozenDeadlock(const char *mapStr) {
		SCOPE(mapStr);
		SimpleDeadlockDetector simple;
		simple.init(s);
		FrozenDeadlockDetector frozen;
		frozen.init(s);
		ASSERT_FALSE(frozen.hasDeadlock(simple, s)) << "Blocked fields: " << getDeadlocks(frozen, s);
	}

	template<class T>
	std::string getDeadlocks(T& detector, BoardState& state) {
		std::string blocked;
		DeadlockSet set;
		detector.fillDeadlocks(set);
		for (int index : set) {
			int col, row;
			state.getColRowFromIndex(index, col, row);
			blocked += std::to_string(col + 1) + ":" + std::to_string(row + 1) + " ";
		}
		return blocked;
	}
};


TEST_F(BoardStateTest, testFillState) {
	{
		const char* mapStr =
			"#####\n"
			"#@$.#\n"
			"#####";
		SCOPE(mapStr);
		ASSERT_EQ(5, s.getWidth());
		ASSERT_EQ(3, s.getHeight());
	}
	{
		const char* mapStr =
			"#####\n"
			"#@$.#\n"
			"#   #\n"
			"#   ####\n"
			"#      #\n"
			"########";
		SCOPE(mapStr);
		ASSERT_EQ(8, s.getWidth());
		ASSERT_EQ(6, s.getHeight());
	}
}

TEST_F(BoardStateTest, testDone) {
	const char* mapStr =
		"############\n"
		"#$$@ #     ###\n"
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
	ASSERT_TRUE(s.isDone()) << "Could not detect the done state in the board\n" << mapStr;
}

TEST_F(BoardStateTest, testSuccessors4) {
	const char* mapStr =
		"############\n"
		"#$$  #     ###\n"
		"#$$  #       #\n"
		"#$$  #@####  #\n"
		"#$$      ##  #\n"
		"#$$  # #    ##\n"
		"###### ##    #\n"
		"#          #\n"
		"#    #     #\n"
		"############";

	SCOPE(mapStr);
	ASSERT_EQ(14, s.getWidth());
	ASSERT_EQ(10, s.getHeight());

	std::vector<int> successors;
	s.getReachableIndices(s.getIndex(4, 4), successors);
	ASSERT_EQ(4, (int)successors.size()) << "Expected to find 4 indices for 4, 4 - but found (" << indicesToColRowString(s, successors) << ")";
}

TEST_F(BoardStateTest, testSuccessors) {
	const char* mapStr =
		"###\n"
		"# #\n"
		"# ##########\n"
		"#          #\n"
		"#    #     #\n"
		"#    #     #\n"
		"############";

	SCOPE(mapStr);
	std::vector<int> successors;
	{
		s.getReachableIndices(s.getIndex(1, 1), successors);
		ASSERT_EQ(1, successors.size()) << "Expected to find 1 indices for 2, 2 - but found (" << indicesToColRowString(s, successors) << ")";
		ASSERT_EQ(s.getIndex(1, 2), successors.front());
		successors.clear();
	}
	{
		s.getReachableIndices(s.getIndex(1, 2), successors);
		const int index1 = s.getIndex(1, 3);
		const int index2 = s.getIndex(1, 1);
		ASSERT_EQ(2, successors.size()) << "Expected to find 2 indices for 2, 3 - but found (" << indicesToColRowString(s, successors) << ")";
		ASSERT_NE(successors[0], successors[1]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_TRUE(index1 == successors[0] || index2 == successors[0]) << "Indices don't match any successor (" << index1 << ", " << index2 << ") - successors (" << successors[0] << ")";
		ASSERT_TRUE(index1 == successors[1] || index2 == successors[1]) << "Indices don't match any successor (" << index1 << ", " << index2 << ") - successors (" << successors[1] << ")";
		successors.clear();
	}
	{
		s.getReachableIndices(s.getIndex(1, 3), successors);
		const int index1 = s.getIndex(1, 4);
		const int index2 = s.getIndex(2, 3);
		const int index3 = s.getIndex(1, 2);
		ASSERT_EQ(3, successors.size()) << "Expected to find 3 indices for 2, 4 - but found (" << indicesToColRowString(s, successors) << ")";
		ASSERT_NE(successors[0], successors[1]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_NE(successors[1], successors[2]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_NE(successors[0], successors[2]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_TRUE(index1 == successors[0] || index2 == successors[0] || index3 == successors[0]) << "Indices don't match any successor (" << index1 << ", " << index2 << ", " << index3 << ") - successors (" << successors[0] << ")";
		ASSERT_TRUE(index1 == successors[1] || index2 == successors[1] || index3 == successors[1]) << "Indices don't match any successor (" << index1 << ", " << index2 << ", " << index3 << ") - successors (" << successors[1] << ")";
		ASSERT_TRUE(index1 == successors[2] || index2 == successors[2] || index3 == successors[2]) << "Indices don't match any successor (" << index1 << ", " << index2 << ", " << index3 << ") - successors (" << successors[2] << ")";
		successors.clear();
	}
	{
		s.getReachableIndices(s.getIndex(2, 4), successors);
		const int index1 = s.getIndex(1, 4);
		const int index2 = s.getIndex(3, 4);
		const int index3 = s.getIndex(2, 3);
		const int index4 = s.getIndex(2, 5);
		ASSERT_EQ(4, successors.size()) << "Expected to find 4 indices for 3, 5 - but found (" << indicesToColRowString(s, successors) << ")";
		ASSERT_NE(successors[0], successors[1]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_NE(successors[1], successors[2]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_NE(successors[2], successors[3]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_NE(successors[0], successors[2]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_NE(successors[0], successors[3]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_NE(successors[1], successors[3]) << "Duplicated successor found: " << vecToString(successors);
		ASSERT_TRUE(index1 == successors[0] || index2 == successors[0] || index3 == successors[0] || index4 == successors[0]) << "Indices don't match any successor (" << index1 << ", " << index2 << ", " << index3 << ", " << index4 << ") - successors (" << successors[0] << ")";
		ASSERT_TRUE(index1 == successors[1] || index2 == successors[1] || index3 == successors[1] || index4 == successors[1]) << "Indices don't match any successor (" << index1 << ", " << index2 << ", " << index3 << ", " << index4 << ") - successors (" << successors[1] << ")";
		ASSERT_TRUE(index1 == successors[2] || index2 == successors[2] || index3 == successors[2] || index4 == successors[2]) << "Indices don't match any successor (" << index1 << ", " << index2 << ", " << index3 << ", " << index4 << ") - successors (" << successors[2] << ")";
		ASSERT_TRUE(index1 == successors[3] || index2 == successors[3] || index3 == successors[3] || index4 == successors[3]) << "Indices don't match any successor (" << index1 << ", " << index2 << ", " << index3 << ", " << index4 << ") - successors (" << successors[3] << ")";
		successors.clear();
	}
	ASSERT_TRUE(s.isDone()) << "Could not detect the done state in the board\n" << mapStr;
}

TEST_F(BoardStateTest, testSimpleDeadlocks) {
	testNoSimpleDeadlock(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n");
	testNoSimpleDeadlock(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n");
	testNoSimpleDeadlockAt(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n",
		2, 2);
	testNoSimpleDeadlockAt(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n",
		1, 3);
	testNoSimpleDeadlockAt(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n",
		4, 2);
	testSimpleDeadlockAt(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n",
		1, 1);
	testSimpleDeadlockAt(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n",
		2, 1);
	testSimpleDeadlockAt(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n",
		3, 1);
	testSimpleDeadlockAt(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n",
		4, 1);
	testNoSimpleDeadlockAt(
		"######\n"
		"#  @ #\n"
		"#$   #\n"
		"#.####\n"
		"###\n",
		1, 2);
	testNoSimpleDeadlock(
		"######\n"
		"#  @ #\n"
		"#  $ #\n"
		"#.####\n"
		"###\n");
	testNoSimpleDeadlock(
		"######\n"
		"#  @ #\n"
		"#$ $.#\n"
		"#.####\n"
		"###\n");
	testSimpleDeadlock(
		"######\n"
		"#$   #\n"
		"#  @.#\n"
		"#.####\n"
		"###\n");
	testSimpleDeadlock(
		"######\n"
		"# $  #\n"
		"#  @.#\n"
		"#.####\n"
		"###\n");
	testSimpleDeadlock(
		"######\n"
		"#   $#\n"
		"#  @.#\n"
		"#.####\n"
		"###\n");
}

TEST_F(BoardStateTest, testDeadlocks) {
	testDeadlock(
		"#####\n"
		"#   #\n"
		"# @ #\n"
		"#$ .#\n"
		"#####");
	testDeadlock(
		"#####\n"
		"#   #\n"
		"#$@ #\n"
		"#  .#\n"
		"#####");
	testDeadlock(
		"#####\n"
		"#$  #\n"
		"# @ #\n"
		"#  .#\n"
		"#####");
	testDeadlock(
		"#####\n"
		"# $ #\n"
		"# @ #\n"
		"#  .#\n"
		"#####");
	testDeadlock(
		"#####\n"
		"#  $#\n"
		"# @ #\n"
		"#  .#\n"
		"#####");
	testNoDeadlock(
		"#####\n"
		"#***#\n"
		"#*@ #\n"
		"#* .#\n"
		"#####");
	testNoDeadlock(
		"#####\n"
		"#@ .#\n"
		"#.$$#\n"
		"# $.#\n"
		"#   #\n"
		"#####");
	testNoDeadlock(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n");
}

TEST_F(BoardStateTest, testNoFrozenDeadlocks) {
	testNoFrozenDeadlock(
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n");
	testNoFrozenDeadlock(
		"############\n"
		"#    #     ###\n"
		"#    #       #\n"
		"#    #@####  #\n"
		"#.       ##  #\n"
		"#    # #    ##\n"
		"###### ##    #\n"
		"  # $        #\n"
		"  #    #     #\n"
		"  ############\n");
	testNoFrozenDeadlock(
		"############\n"
		"#    #     ###\n"
		"#    #       #\n"
		"#    #@####  #\n"
		"#.       ##  #\n"
		"#    # #    ##\n"
		"###### ##    #\n"
		"  #    $     #\n"
		"  #    #     #\n"
		"  ############\n");
	testNoFrozenDeadlock(
		"############\n"
		"#    #     ###\n"
		"#    #       #\n"
		"#    #@####  #\n"
		"#.       ##  #\n"
		"#    # #    ##\n"
		"###### ##  $ #\n"
		"  #          #\n"
		"  #    #     #\n"
		"  ############\n");
	testNoFrozenDeadlock(
		"############\n"
		"#    #     ###\n"
		"#    #       #\n"
		"#.   #@####  #\n"
		"#.       ##  #\n"
		"#    # #    ##\n"
		"###### ##  $ #\n"
		"  #        $ #\n"
		"  #    #     #\n"
		"  ############\n");
	testNoFrozenDeadlock(
		"############\n"
		"#    #     ###\n"
		"#    #       #\n"
		"#    #@####  #\n"
		"#.       ##  #\n"
		"#    # #    ##\n"
		"###### ##$   #\n"
		"  #          #\n"
		"  #    #     #\n"
		"  ############\n");
	testNoFrozenDeadlock(
		"############\n"
		"#    #     ###\n"
		"#    #       #\n"
		"#.   #@####  #\n"
		"#.       ##  #\n"
		"#    # #    ##\n"
		"###### ##$   #\n"
		"  #      $   #\n"
		"  #    #     #\n"
		"  ############\n");
	testNoFrozenDeadlock(
		"############\n"
		"#    #     ###\n"
		"#    #$$  $  #\n"
		"#.   #@####  #\n"
		"#.       ##  #\n"
		"#    # #    ##\n"
		"###### ##    #\n"
		"  #          #\n"
		"  #    #     #\n"
		"  ############\n");
	testNoFrozenDeadlock(
		"############\n"
		"#..  #     ###\n"
		"#..  #$$  $  #\n"
		"#..  #@####  #\n"
		"#..      ##  #\n"
		"#..  # #  $ ##\n"
		"###### ##$ $ #\n"
		"  # $  $ $ $ #\n"
		"  #    #     #\n"
		"  ############\n");
}

TEST_F(BoardStateTest, testNoDeadlockButBlockedPackages) {
	const char* mapStr =
		"######\n"
		"#    #\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n";
	testNoDeadlock(mapStr);
}


TEST_F(BoardStateTest, testDeadlockByBlockedPackages) {
	const char* mapStr =
		"######\n"
		"#$$@.#\n"
		"#.####\n"
		"###\n";
	testDeadlock(mapStr);
}

TEST_F(BoardStateTest, testDeadlockNoDeadlock) {
	// xsokoban2 state that
	const char* mapStr =
		"############\n"
		"#..  #     ###\n"
		"#..  #$$  $  #\n"
		"#..  #@####  #\n"
		"#..      ##  #\n"
		"#..  # #  $ ##\n"
		"###### ##$ $ #\n"
		"  # $  $ $ $ #\n"
		"  #    #     #\n"
		"  ############\n";
	testNoDeadlock(mapStr);
}

TEST_F(BoardStateTest, testDeadlockDeadlockFound1) {
	const char* mapStr =
		"    #####\n"
		"    #@  #\n"
		"    #   #\n"
		"  ###   ##\n"
		"  #      #\n"
		"### #$## #   ######\n"
		"#   #$## #####  ..#\n"
		"#               ..#\n"
		"##### ### # ##  ..#\n"
		"    #     #########\n"
		"    #######\n";
	testDeadlock(mapStr);
}

TEST_F(BoardStateTest, DISABLED_testDeadlockDeadlockFoundCorral) {
	testDeadlock(
		"    #####\n"
		"    #   #\n"
		"    #   #\n"
		"  ###   ##\n"
		"  #      #\n"
		"### #$## #   ######\n"
		"#   # ## #####  ..#\n"
		"#    $   $      ..#\n"
		"#####@### # ##  ..#\n"
		"    #     #########\n"
		"    ######\n");
	testDeadlock(
		"    #####\n"
		"    #@  #\n"
		"    #   #\n"
		"  ###   ##\n"
		"  #      #\n"
		"### #$## #   ######\n"
		"#   # ## #####  ..#\n"
		"#    $  $       ..#\n"
		"##### ### # ##  ..#\n"
		"    #     #########\n"
		"    #######\n");
}

TEST_F(BoardStateTest, testDeadlockDeadlockFound4) {
	const char* mapStr =
		"        ########\n"
		"        #@     #\n"
		"        #  #  ##\n"
		"        #     #\n"
		"        ##$   #\n"
		"######### $$# ###\n"
		"#....  ##  $ $  #\n"
		"##...        $  #\n"
		"#....  ##########\n"
		"########\n";
	testDeadlock(mapStr);
}

TEST_F(BoardStateTest, testDeadlockDeadlockFound5) {
	const char* mapStr =
		"        ########\n"
		"        #@     #\n"
		"        #  #  ##\n"
		"        #     #\n"
		"        ##$   #\n"
		"######### $$#$###\n"
		"#....  ##  $ $  #\n"
		"##...           #\n"
		"#....  ##########\n"
		"########\n";
	testDeadlock(mapStr);
}

TEST_F(BoardStateTest, testDeadlockDeadlockFound6) {
	const char* mapStr =
		"############\n"
		"#..@ #     ###\n"
		"#..  #       #\n"
		"#..  # ####  #\n"
		"#..      ##  #\n"
		"#..  # #  $$##\n"
		"######$##$ $ #\n"
		"  #   $  $   #\n"
		"  #    #     #\n"
		"  ############\n";
	testDeadlock(mapStr);
}

TEST_F(BoardStateTest, testSpeed) {
	std::string mapStr;
	const int maxRow = 512;
	const int maxCol = 512;
	mapStr.reserve(maxCol * maxRow);
	int packages = 0;
	for (int row = 0; row < maxRow; ++row) {
		for (int col = 0; col < maxCol; ++col) {
			// build the outer rectangular wall
			if (col == 0 || col == maxCol - 1 || row == 0 || row == maxRow - 1) {
				mapStr.append("#");
			} else {
				if (col % 5 <= 2 && row % 5 <= 2 && col < maxCol - 2 && row < maxRow - 2) {
					mapStr.append("$");
					mapStr.append(".");
					++packages;
					++col;
				}
			}
		}
		mapStr.append("\n");
	}
	testDeadlock(mapStr.c_str());
}

}
