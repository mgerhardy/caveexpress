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

bool FrozenDeadlockDetector::hasAnyPackageClose(BoardState& s, int col, int row, char dir) const {
	int x;
	int y;
	getXY(dir, x, y);
	const char field = s.getField(col + x, row + y);
	// TODO: check recursivly whether the package is blocked - and convert the package into a wall
	return isPackage(field) || isPackageOnTarget(field);
}

bool FrozenDeadlockDetector::hasSimpleDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s, int col, int row, char dir) const {
	int x;
	int y;
	getXY(dir, x, y);
	const int index = s.getIndex(col + x, row + y);
	return simple.hasDeadlockAt(index);
}

bool FrozenDeadlockDetector::hasDeadlock_(const SimpleDeadlockDetector& simple, BoardState& s, int col, int row) const {
	// a wall on both sides (left/right and up/down)
	const bool blockedHorizontally = hasWallClose(s, col, row, MOVE_LEFT) || hasWallClose(s, col, row, MOVE_RIGHT);
	const bool blockedVertically = hasWallClose(s, col, row, MOVE_UP) || hasWallClose(s, col, row, MOVE_DOWN);
	if (blockedHorizontally && blockedVertically) {
		return true;
	}

	// a simple deadlock square on both sides (left/right and up/down)
	const bool simpleHorizontally = hasSimpleDeadlock(simple, s, col, row, MOVE_LEFT) || hasSimpleDeadlock(simple, s, col, row, MOVE_RIGHT);
	const bool simpleVertically = hasSimpleDeadlock(simple, s, col, row, MOVE_UP) || hasSimpleDeadlock(simple, s, col, row, MOVE_DOWN);
	if (simpleHorizontally && simpleVertically) {
		return true;
	}

	// a package on the side (left/right and up/down) => blocked if the package on the side is also blocked
	const bool packageHorizontally = hasAnyPackageClose(s, col, row, MOVE_LEFT) || hasAnyPackageClose(s, col, row, MOVE_RIGHT);
	const bool packageVertically = hasAnyPackageClose(s, col, row, MOVE_UP) || hasAnyPackageClose(s, col, row, MOVE_DOWN);
	if (packageHorizontally && packageVertically) {
		return true;
	}

	return false;
}

bool FrozenDeadlockDetector::hasDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s) {
	clear();
	for (auto i = s.begin(); i != s.end(); ++i) {
		if (!isPackage(i->second)) {
			continue;
		}

		int col;
		int row;
		if (!s.getColRowFromIndex(i->first, col, row))
			continue;

		BoardState copy = s;
		if (hasDeadlock_(simple, copy, col, row)) {
			_deadlocks.insert(i->first);
			return true;
		}
	}
	return false;
}

}
