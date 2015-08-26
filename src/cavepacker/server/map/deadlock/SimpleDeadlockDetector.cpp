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
	for (auto i = s.begin(); i != s.end(); ++i) {
		if (!isTarget(i->second) && !isPackageOnTarget(i->second)) {
			continue;
		}
		targets.push_back(i->first);
	}
	Log::debug(LOG_SERVER, "Found %i targets", (int)targets.size());

	BoardState copy = s;
	for (auto i = copy.begin(); i != copy.end(); ++i) {
		if (isPackage(i->second)) {
			copy.clearFieldForIndex(i->first);
			copy.setFieldForIndex(i->first, Sokoban::GROUND);
			Log::debug(LOG_SERVER, "replaced package with ground at %i", i->first);
		} else if (isPackageOnTarget(i->second)) {
			copy.clearFieldForIndex(i->first);
			copy.setFieldForIndex(i->first, Sokoban::TARGET);
			Log::debug(LOG_SERVER, "replaced packageontarget with target at %i", i->first);
		}
	}
	Log::debug(LOG_SERVER, "board state:\n%s", copy.toString().c_str());

	for (int targetIndex : targets) {
		moveBackwards(copy, targetIndex);
	}

	for (auto i = s.begin(); i != s.end(); ++i) {
		if (!isGround(i->second) && !isPackage(i->second)) {
			continue;
		}
		if (_visited.find(i->first) != _visited.end()) {
			continue;
		}
		int col;
		int row;
		s.getColRowFromIndex(i->first, col, row);
		Log::debug(LOG_SERVER, "Simple deadlock detected at %i:%i", col, row);
		_deadlocks.insert(i->first);
	}
	Log::debug(LOG_SERVER, "Found %i simple deadlocks", (int)_deadlocks.size());
	Log::debug(LOG_SERVER, "Visited %i fields", (int)_visited.size());
	_visited.clear();
}

bool SimpleDeadlockDetector::hasDeadlock(const BoardState& s) const {
	for (auto i = s.begin(); i != s.end(); ++i) {
		if (!isPackage(i->second)) {
			continue;
		}
		if (_deadlocks.find(i->first) != _deadlocks.end()) {
			return true;
		}
	}
	return false;
}

}
