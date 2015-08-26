#include "FrozenDeadlockDetector.h"
#include "SimpleDeadlockDetector.h"
#include "cavepacker/server/map/BoardState.h"
#include <SDL_assert.h>

namespace cavepacker {

bool FrozenDeadlockDetector::hasWallClose(const BoardState& s, int col, int row, char dir) const {
	int x;
	int y;
	getXY(dir, x, y);
	const char field = s.getField(col + x, row + y);
	return isWall(field);
}

bool FrozenDeadlockDetector::hasSimpleDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s, int col, int row, char dir) {
	int x;
	int y;
	getXY(dir, x, y);
	const int index = s.getIndex(col + x, row + y);
	return simple.hasDeadlockAt(index);
}

bool FrozenDeadlockDetector::hasDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s) {
	for (auto i = s.begin(); i != s.end(); ++i) {
		if (!isPackage(i->second)) {
			continue;
		}

		int col;
		int row;
		if (!s.getColRowFromIndex(i->first, col, row))
			continue;

		// a wall on both sides (left/right and up/down)
		const bool blockedHorizontally = hasWallClose(s, col, row, MOVE_LEFT) || hasWallClose(s, col, row, MOVE_RIGHT);
		const bool blockedVertically = hasWallClose(s, col, row, MOVE_UP) || hasWallClose(s, col, row, MOVE_DOWN);
		if (blockedHorizontally && blockedVertically) {
			_deadlocks.insert(i->first);
			return true;
		}

		// a simple deadlock square on both sides (left/right and up/down)
		const bool simpleHorizontally = hasSimpleDeadlock(simple, s, col, row, MOVE_LEFT) || hasSimpleDeadlock(simple, s, col, row, MOVE_RIGHT);
		const bool simpleVertically = hasSimpleDeadlock(simple, s, col, row, MOVE_UP) || hasSimpleDeadlock(simple, s, col, row, MOVE_DOWN);
		if (simpleHorizontally && simpleVertically) {
			_deadlocks.insert(i->first);
			return true;
		}

		// a package on the side (left/right and up/down) => blocked if the package on the side is also blocked
		// TODO:
	}
	return false;
}

}
