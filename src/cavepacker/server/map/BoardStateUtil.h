#pragma once

#include "cavepacker/server/map/BoardState.h"
#include "cavepacker/server/map/SokobanTiles.h"

namespace cavepacker {

static bool createBoardStateFromString(BoardState& s, const char* board, bool convertPlayers = true) {
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
			if (!s.setField(col, row, c))
				return false;
			++col;
		}
		++d;
	}
	// at least 3 rows are needed
	if (row < 2)
		return false;
	s.initDeadlock();
	return true;
}

}
