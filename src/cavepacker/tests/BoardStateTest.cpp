#include "tests/TestShared.h"
#include "cavepacker/server/map/BoardState.h"
#include "cavepacker/server/map/deadlock/DeadlockDetector.h"

namespace cavepacker {

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
		BoardState s;
		fillState(s, mapStr);
		ASSERT_TRUE(s.hasDeadlock());
	}

	void testNoDeadlock(const char *mapStr) {
		BoardState s;
		fillState(s, mapStr);
		ASSERT_FALSE(s.hasDeadlock());
	}
};


TEST_F(BoardStateTest, testFillState) {
	{
		BoardState s;
		const char* mapStr =
				"#####\n"
				"#@$.#\n"
				"#####";
		fillState(s, mapStr);
		ASSERT_EQ(5, s.getWidth());
		ASSERT_EQ(3, s.getHeight());
	}
	{
		BoardState s;
		const char* mapStr =
			"#####\n"
			"#@$.#\n"
			"#   #\n"
			"#   ####\n"
			"#      #\n"
			"########";
		fillState(s, mapStr);
		ASSERT_EQ(8, s.getWidth());
		ASSERT_EQ(6, s.getHeight());
	}
}

TEST_F(BoardStateTest, testDone) {
	BoardState s;

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

	fillState(s, mapStr);
	ASSERT_EQ(14, s.getWidth());
	ASSERT_EQ(10, s.getHeight());
	ASSERT_TRUE(s.isDone()) << "Could not detect the done state in the board\n" << mapStr;
}

TEST_F(BoardStateTest, testDeadlockSimple1) {
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

TEST_F(BoardStateTest, testDeadlockDeadlockFound2) {
	const char* mapStr =
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
		"    ######\n";
	testDeadlock(mapStr);
}

TEST_F(BoardStateTest, testDeadlockDeadlockFound3) {
	const char* mapStr =
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
		"    #######\n";
	testDeadlock(mapStr);
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

}
