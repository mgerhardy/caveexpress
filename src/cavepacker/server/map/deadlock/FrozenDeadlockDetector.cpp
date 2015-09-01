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
	if (isPackage(field) || isPackageOnTarget(field)) {
		s.clearField(col, row);
		s.setField(col, row, Sokoban::WALL);
		const bool dl = hasDeadlock(simple, s);
		s.clearField(col, row);
		s.setField(col, row, field);
		return dl;
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

bool FrozenDeadlockDetector::hasDeadlockVertically(const SimpleDeadlockDetector& simple, BoardState& s, int col, int row) {
	const bool blockedVertically = HASWALL(MOVE_UP) || HASWALL(MOVE_DOWN);
	if (blockedVertically)
		return true;
	const bool simpleVertically = SIMPLEDEADLOCK(MOVE_UP) && SIMPLEDEADLOCK(MOVE_DOWN);
	if (simpleVertically)
		return true;
	const bool packageVertically = BLOCKEDPACKAGE(MOVE_UP) || BLOCKEDPACKAGE(MOVE_DOWN);
	if (packageVertically)
		return true;

	return false;
}

bool FrozenDeadlockDetector::hasDeadlock_(const SimpleDeadlockDetector& simple, BoardState& s, int col, int row) {
	const bool blockedHorizontally = HASWALL(MOVE_LEFT) || HASWALL(MOVE_RIGHT);
	if (blockedHorizontally) {
		const bool blockedVertically = hasDeadlockVertically(simple, s, col, row);
		return blockedVertically;
	}
	const bool simpleHorizontally = SIMPLEDEADLOCK(MOVE_LEFT) && SIMPLEDEADLOCK(MOVE_RIGHT);
	if (simpleHorizontally) {
		const bool blockedVertically = hasDeadlockVertically(simple, s, col, row);
		return blockedVertically;
	}
	const bool packageHorizontally = BLOCKEDPACKAGE(MOVE_LEFT) || BLOCKEDPACKAGE(MOVE_RIGHT);
	if (packageHorizontally) {
		const bool blockedVertically = hasDeadlockVertically(simple, s, col, row);
		return blockedVertically;
	}

	return false;
}

bool FrozenDeadlockDetector::hasDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s) {
	clear();
	BoardState copy = s;
	for (auto i = s.begin(); i != s.end(); ++i) {
		if (!isPackage(i->second)) {
			continue;
		}

		int col;
		int row;
		if (!s.getColRowFromIndex(i->first, col, row))
			continue;

		if (hasDeadlock_(simple, copy, col, row)) {
			_deadlocks.insert(i->first);
			return true;
		}
	}
	return false;
}

}
