#include "SimpleDeadlockDetector.h"
#include "cavepacker/server/map/SokobanTiles.h"
#include "cavepacker/server/map/BoardState.h"

namespace cavepacker {

bool SimpleDeadlockDetector::pull(char direction, BoardState& s, int index) {
	int col;
	int row;
	if (!s.getColRowFromIndex(index, col, row))
		return false;

	int x;
	int y;
	getXY(direction, x, y);
	// we need two fields to be free in the back of the package - one for the player to stand on
	// and one for the field where the player would move to
	if (s.isFree(col + x, row + y) && s.isFree(col + x + x, row + y + y)) {
		Log::debug(LOG_SERVER, "col: %i, row: %i now has a package", col + x, row + y);
		const int newIndex = s.getIndex(col + x, row + y);
		if (_visited.find(newIndex) != _visited.end()) {
			return true;
		}
		_visited.insert(newIndex);
		moveBackwards(s, newIndex);
		return true;
	}
	return false;
}

bool SimpleDeadlockDetector::moveBackwards(BoardState& s, int index) {
	bool moved = false;
	if (pull(MOVE_LEFT, s, index))
		moved = true;
	if (pull(MOVE_RIGHT, s, index))
		moved = true;
	if (pull(MOVE_UP, s, index))
		moved = true;
	if (pull(MOVE_DOWN, s, index))
		moved = true;
	return moved;
}

void SimpleDeadlockDetector::init(const BoardState& s) {
	std::vector<int> targets;
	int index = 0;
	for (auto i = s.begin(); i != s.end(); ++i, ++index) {
		if (!isTarget(*i) && !isPackageOnTarget(*i)) {
			continue;
		}
		targets.push_back(index);
	}
	Log::debug(LOG_SERVER, "Found %i targets", (int)targets.size());

	BoardState copy(s);
	index = 0;
	for (auto i = s.begin(); i != s.end(); ++i, ++index) {
		const char field = *i;
		if (isPackage(field)) {
			copy.clearFieldForIndex(index);
			copy.setFieldForIndex(index, Sokoban::GROUND);
			Log::debug(LOG_SERVER, "replaced package with ground at %i", index);
		} else if (isPackageOnTarget(field)) {
			copy.clearFieldForIndex(index);
			copy.setFieldForIndex(index, Sokoban::TARGET);
			Log::debug(LOG_SERVER, "replaced packageontarget with target at %i", index);
		}
	}
#ifdef DEBUG
	Log::debug(LOG_SERVER, "board state:\n%s", copy.toString().c_str());
#endif

	for (int targetIndex : targets) {
		moveBackwards(copy, targetIndex);
	}

	index = 0;
	for (auto i = copy.begin(); i != copy.end(); ++i, ++index) {
		const char field = *i;
		if (!isGround(field) && !isPackage(field)) {
			continue;
		}
		if (_visited.find(index) != _visited.end()) {
			continue;
		}
#ifdef DEBUG
		int col;
		int row;
		s.getColRowFromIndex(index, col, row);
		Log::debug(LOG_SERVER, "Simple deadlock detected at %i:%i", col, row);
#endif
		_deadlocks.insert(index);
	}
	Log::debug(LOG_SERVER, "Found %i simple deadlocks", (int)_deadlocks.size());
	Log::debug(LOG_SERVER, "Visited %i fields", (int)_visited.size());
	_visited.clear();
}

bool SimpleDeadlockDetector::hasDeadlock(uint32_t millisStart, uint32_t millisTimeout, const BoardState& s) const {
	int index = 0;
	for (auto i = s.begin(); i != s.end(); ++i, ++index) {
		TIMEOUTREACHED(millisStart + millisTimeout)
		if (!isPackage(*i)) {
			continue;
		}
		if (_deadlocks.find(index) != _deadlocks.end()) {
			return true;
		}
	}
	return false;
}

}
