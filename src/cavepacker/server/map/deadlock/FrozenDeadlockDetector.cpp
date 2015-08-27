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

bool FrozenDeadlockDetector::hasAnyBlockedPackageClose(const SimpleDeadlockDetector& simple, BoardState& s, int col, int row, char dir) {
	int x;
	int y;
	getXY(dir, x, y);
	const char field = s.getField(col + x, row + y);
	// TODO: check recursively whether the package is blocked - and convert the package into a wall
	if (isPackage(field) || isPackageOnTarget(field)) {
		BoardState copy = s;
		copy.clearField(col, row);
		copy.setField(col, row, Sokoban::WALL);
		return hasDeadlock(simple, copy);
	}
	return false;
}

bool FrozenDeadlockDetector::hasSimpleDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s, int col, int row, char dir) const {
	int x;
	int y;
	getXY(dir, x, y);
	const int index = s.getIndex(col + x, row + y);
	return simple.hasDeadlockAt(index);
}

#define BLOCKEDPACKAGE(dir) hasAnyBlockedPackageClose(simple, s, col, row, dir)
#define HASWALL(dir) hasWallClose(s, col, row, dir)
#define SIMPLEDEADLOCK(dir) hasSimpleDeadlock(simple, s, col, row, dir)

bool FrozenDeadlockDetector::hasDeadlock_(const SimpleDeadlockDetector& simple, BoardState& s, int col, int row) {
	// a wall on both sides (left/right and up/down)
	const bool blockedHorizontally = HASWALL(MOVE_LEFT) || HASWALL(MOVE_RIGHT);
	const bool blockedVertically = HASWALL(MOVE_UP) || HASWALL(MOVE_DOWN);
	if (blockedHorizontally && blockedVertically) {
		return true;
	}

	// a simple deadlock square on both sides (left/right and up/down)
	const bool simpleHorizontally = SIMPLEDEADLOCK(MOVE_LEFT) || SIMPLEDEADLOCK(MOVE_RIGHT);
	const bool simpleVertically = SIMPLEDEADLOCK(MOVE_UP) || SIMPLEDEADLOCK(MOVE_DOWN);
	if (simpleHorizontally && simpleVertically) {
		return true;
	}

	// a package on the side (left/right and up/down) => blocked if the package on the side is also blocked
	const bool packageHorizontally = BLOCKEDPACKAGE(MOVE_LEFT) || BLOCKEDPACKAGE(MOVE_RIGHT);
	const bool packageVertically = BLOCKEDPACKAGE(MOVE_UP) || BLOCKEDPACKAGE(MOVE_DOWN);
	if (packageHorizontally && packageVertically) {
		return true;
	}

	const bool combinedHorizontally = blockedHorizontally | simpleHorizontally | packageHorizontally;
	const bool combinedVertically = blockedVertically | simpleVertically | packageVertically;
	if (combinedHorizontally && combinedVertically) {
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
