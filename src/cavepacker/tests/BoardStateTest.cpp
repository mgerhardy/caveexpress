#include "tests/TestShared.h"
#include "common/Common.h"
#include "cavepacker/server/map/BoardState.h"
#include "cavepacker/server/map/deadlock/DeadlockDetector.h"

namespace cavepacker {

#define SCOPE(mapStr) BoardState s; fillState(s, (mapStr)); SCOPED_TRACE(va("%s\n%s", __PRETTY_FUNCTION__, s.toString().c_str()))

class BoardStateTest: public AbstractTest {
protected:
	void fillState(BoardState& s, const char* board, bool convertPlayers = true) const {
		int col = 0;
		int row = 0;
		int maxCol = 0;
		const char *d = board;
		while (*d != '\0') {
			if (*d == '\n') {
				if (*(d + 1) == '\0')
					break;
				++row;
				maxCol = std::max(maxCol, col);
				col = 0;
			} else {
				++col;
			}
			++d;
		}
		// we need the size for proper index calculations
		s.setSize(maxCol, row + 1);
		// after finding out the size - let's fill the board
		d = board;
		col = 0;
		row = 0;
		while (*d != '\0') {
			if (*d == '\n') {
				if (*(d + 1) == '\0')
					break;
				++row;
				col = 0;
			} else {
				char c = *d;
				// usually other players block the movement, but for the test we just ignore this
				if (convertPlayers && (c == Sokoban::PLAYER || c == Sokoban::PLAYERONTARGET))
					c = Sokoban::GROUND;
				ASSERT_TRUE(s.setField(col, row, c)) << "Could not set the field " << c << " at " << col << ":" << row;
				++col;
			}
			++d;
		}
		// at least 3 rows are needed
		ASSERT_GE(row, 2) << "could not fill the board state with " << board;
		s.initDeadlock();
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
		ASSERT_FALSE(simple.hasDeadlockAt(s.getIndex(col, row))) << "Blocked fields: " << getDeadlocks(simple, s);
	}

	void testSimpleDeadlockAt(const char *mapStr, int col, int row) {
		SCOPE(mapStr);
		SimpleDeadlockDetector simple;
		simple.init(s);
		ASSERT_TRUE(simple.hasDeadlockAt(s.getIndex(col, row))) << "Blocked fields: " << getDeadlocks(simple, s);
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

TEST_F(BoardStateTest, testDeadlockDeadlockFoundCorral) {
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
				if (col % 5 == 0 && row % 5 == 0 && col < maxCol - 2 && row < maxRow - 2) {
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
