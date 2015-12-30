#include "FrozenDeadlockDetector.h"
#include "SimpleDeadlockDetector.h"
#include "cavepacker/shared/BoardState.h"
#include <SDL_assert.h>

namespace cavepacker {

bool FrozenDeadlockDetector::hasWallClose(const BoardState& s, int index) const {
	const char field = s.getFieldByIndex(index);
	return isWall(field);
}

bool FrozenDeadlockDetector::hasAnyBlockedPackageClose(uint32_t millisStart, uint32_t millisTimeout, const SimpleDeadlockDetector& simple, BoardState& s, int index, int origIndex) {
	const char field = s.getFieldByIndex(index);
	if (isPackage(field) || isPackageOnTarget(field)) {
		s.clearFieldForIndex(origIndex);
		s.setFieldForIndex(origIndex, Sokoban::WALL);
		const bool dl = hasDeadlock(millisStart, millisTimeout, simple, s);
		s.clearFieldForIndex(origIndex);
		s.setFieldForIndex(origIndex, field);
		return dl;
	}
	return false;
}

bool FrozenDeadlockDetector::hasSimpleDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s, int index) const {
	return simple.hasDeadlockAt(index);
}

#define BLOCKEDPACKAGE(indexPrefix) hasAnyBlockedPackageClose(millisStart, millisTimeout, simple, s, index##indexPrefix, origIndex)
#define HASWALL(indexPrefix) hasWallClose(s, index##indexPrefix)
#define SIMPLEDEADLOCK(indexPrefix) hasSimpleDeadlock(simple, s, index##indexPrefix)

bool FrozenDeadlockDetector::hasDeadlockVertically(uint32_t millisStart, uint32_t millisTimeout, const SimpleDeadlockDetector& simple, BoardState& s, int col, int row) {
	int upx;
	int upy;
	getXY(MOVE_UP, upx, upy);
	const int indexUp = s.getIndex(col + upx, row + upy);

	int downx;
	int downy;
	getXY(MOVE_DOWN, downx, downy);
	const int indexDown = s.getIndex(col + downx, row + downy);

	const bool blockedVertically = HASWALL(Up) || HASWALL(Down);
	if (blockedVertically)
		return true;
	const bool simpleVertically = SIMPLEDEADLOCK(Up) && SIMPLEDEADLOCK(Down);
	if (simpleVertically)
		return true;

	const int origIndex = s.getIndex(col, row);
	const bool packageVertically = BLOCKEDPACKAGE(Up) || BLOCKEDPACKAGE(Down);
	if (packageVertically)
		return true;

	return false;
}

bool FrozenDeadlockDetector::hasDeadlock_(uint32_t millisStart, uint32_t millisTimeout, const SimpleDeadlockDetector& simple, BoardState& s, int col, int row) {
	int rightx;
	int righty;
	getXY(MOVE_RIGHT, rightx, righty);
	const int indexRight = s.getIndex(col + rightx, row + righty);

	int leftx;
	int lefty;
	getXY(MOVE_LEFT, leftx, lefty);
	const int indexLeft = s.getIndex(col + leftx, row + lefty);

	const bool blockedHorizontally = HASWALL(Left) || HASWALL(Right);
	if (blockedHorizontally) {
		const bool blockedVertically = hasDeadlockVertically(millisStart, millisTimeout, simple, s, col, row);
		return blockedVertically;
	}
	const bool simpleHorizontally = SIMPLEDEADLOCK(Left) && SIMPLEDEADLOCK(Right);
	if (simpleHorizontally) {
		const bool blockedVertically = hasDeadlockVertically(millisStart, millisTimeout, simple, s, col, row);
		return blockedVertically;
	}
	const int origIndex = s.getIndex(col, row);
	const bool packageHorizontally = BLOCKEDPACKAGE(Left) || BLOCKEDPACKAGE(Right);
	if (packageHorizontally) {
		const bool blockedVertically = hasDeadlockVertically(millisStart, millisTimeout, simple, s, col, row);
		return blockedVertically;
	}

	return false;
}

bool FrozenDeadlockDetector::hasDeadlock(uint32_t millisStart, uint32_t millisTimeout, const SimpleDeadlockDetector& simple, const BoardState& s) {
	clear();
	BoardState copy = s;
	int index = 0;
	for (auto i = s.begin(); i != s.end(); ++i, ++index) {
		TIMEOUTREACHED(millisStart + millisTimeout)
		if (!isPackage(*i)) {
			continue;
		}

		int col;
		int row;
		if (!s.getColRowFromIndex(index, col, row))
			continue;

		if (hasDeadlock_(millisStart, millisTimeout, simple, copy, col, row)) {
			_deadlocks.insert(index);
			return true;
		}
	}
	return false;
}

}
