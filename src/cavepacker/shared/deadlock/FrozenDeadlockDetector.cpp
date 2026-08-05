#include "FrozenDeadlockDetector.h"
#include "SimpleDeadlockDetector.h"
#include "cavepacker/shared/BoardState.h"
#include <SDL_assert.h>

namespace cavepacker {

bool FrozenDeadlockDetector::hasWallClose(const BoardState& s, int index) const {
	return isWall(s.getFieldByIndex(index));
}

bool FrozenDeadlockDetector::hasSimpleDeadlock(const SimpleDeadlockDetector& simple, int index) const {
	return simple.hasDeadlockAt(index);
}

bool FrozenDeadlockDetector::isPackageFrozen(uint32_t millisStart, uint32_t millisTimeout,
		const SimpleDeadlockDetector& simple, BoardState& s,
		std::vector<uint8_t>& visiting, int index) {
	if (index < 0 || index >= (int)visiting.size())
		return false;
	if (visiting[index])
		return true;

	int col;
	int row;
	if (!s.getColRowFromIndex(index, col, row))
		return false;

	visiting[index] = 1;
	const bool frozen = hasDeadlockAt(millisStart, millisTimeout, simple, s, visiting, col, row);
	visiting[index] = 0;
	return frozen;
}

bool FrozenDeadlockDetector::hasBlockedPackageClose(uint32_t millisStart, uint32_t millisTimeout,
		const SimpleDeadlockDetector& simple, BoardState& s,
		std::vector<uint8_t>& visiting, int neighborIndex, int origIndex) {
	const char field = s.getFieldByIndex(neighborIndex);
	if (!isPackage(field) && !isPackageOnTarget(field))
		return false;

	// Same as before: treat the current package as a wall and see whether that
	// creates a frozen deadlock anywhere. Use a visiting set instead of calling
	// hasDeadlock() again (which used to re-copy the board and rescan everything).
	const char restored = s.clearFieldForIndex(origIndex);
	s.setFieldForIndex(origIndex, Sokoban::WALL);

	bool deadlock = false;
	int index = 0;
	int checked = 0;
	const uint32_t deadline = millisStart + millisTimeout;
	for (auto i = s.begin(); i != s.end(); ++i, ++index) {
		if ((++checked & 7) == 0) {
			if (SDL_TICKS_PASSED(SDL_GetTicks(), deadline)) {
				s.clearFieldForIndex(origIndex);
				s.setFieldForIndex(origIndex, restored);
				return false;
			}
		}
		if (!isPackage(*i))
			continue;
		if (isPackageFrozen(millisStart, millisTimeout, simple, s, visiting, index)) {
			deadlock = true;
			break;
		}
	}

	s.clearFieldForIndex(origIndex);
	s.setFieldForIndex(origIndex, restored);
	return deadlock;
}

bool FrozenDeadlockDetector::hasDeadlockVertically(uint32_t millisStart, uint32_t millisTimeout,
		const SimpleDeadlockDetector& simple, BoardState& s,
		std::vector<uint8_t>& visiting, int col, int row) {
	int upx, upy;
	getXY(MOVE_UP, upx, upy);
	const int indexUp = s.getIndex(col + upx, row + upy);

	int downx, downy;
	getXY(MOVE_DOWN, downx, downy);
	const int indexDown = s.getIndex(col + downx, row + downy);

	if (hasWallClose(s, indexUp) || hasWallClose(s, indexDown))
		return true;
	if (hasSimpleDeadlock(simple, indexUp) && hasSimpleDeadlock(simple, indexDown))
		return true;

	const int origIndex = s.getIndex(col, row);
	if (hasBlockedPackageClose(millisStart, millisTimeout, simple, s, visiting, indexUp, origIndex)
			|| hasBlockedPackageClose(millisStart, millisTimeout, simple, s, visiting, indexDown, origIndex))
		return true;
	return false;
}

bool FrozenDeadlockDetector::hasDeadlockAt(uint32_t millisStart, uint32_t millisTimeout,
		const SimpleDeadlockDetector& simple, BoardState& s,
		std::vector<uint8_t>& visiting, int col, int row) {
	int rightx, righty;
	getXY(MOVE_RIGHT, rightx, righty);
	const int indexRight = s.getIndex(col + rightx, row + righty);

	int leftx, lefty;
	getXY(MOVE_LEFT, leftx, lefty);
	const int indexLeft = s.getIndex(col + leftx, row + lefty);

	if (hasWallClose(s, indexLeft) || hasWallClose(s, indexRight)) {
		return hasDeadlockVertically(millisStart, millisTimeout, simple, s, visiting, col, row);
	}
	if (hasSimpleDeadlock(simple, indexLeft) && hasSimpleDeadlock(simple, indexRight)) {
		return hasDeadlockVertically(millisStart, millisTimeout, simple, s, visiting, col, row);
	}

	const int origIndex = s.getIndex(col, row);
	if (hasBlockedPackageClose(millisStart, millisTimeout, simple, s, visiting, indexLeft, origIndex)
			|| hasBlockedPackageClose(millisStart, millisTimeout, simple, s, visiting, indexRight, origIndex)) {
		return hasDeadlockVertically(millisStart, millisTimeout, simple, s, visiting, col, row);
	}
	return false;
}

bool FrozenDeadlockDetector::hasDeadlock(uint32_t millisStart, uint32_t millisTimeout,
		const SimpleDeadlockDetector& simple, const BoardState& s) {
	clear();
	const int size = s.size();
	if (size <= 0)
		return false;

	BoardState copy = s;
	std::vector<uint8_t> visiting(size, 0);

	const uint32_t deadline = millisStart + millisTimeout;
	int index = 0;
	int checked = 0;
	for (auto i = s.begin(); i != s.end(); ++i, ++index) {
		if ((++checked & 7) == 0) {
			TIMEOUTREACHED(deadline)
		}
		if (!isPackage(*i))
			continue;

		std::fill(visiting.begin(), visiting.end(), 0);
		if (isPackageFrozen(millisStart, millisTimeout, simple, copy, visiting, index)) {
			_deadlocks.insert(index);
			return true;
		}
	}
	return false;
}

}
