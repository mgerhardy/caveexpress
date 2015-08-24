#include "DeadlockDetector.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/server/map/SokobanTiles.h"

namespace cavepacker {

StateMap DeadlockDetector::calculateDeadlockFields(DeadlockState& s) {
	StateMap deadlocks;

	BoardState& state = s.state;

	Log::info(LOG_MAP, "calculate deadlock fields %i", state.size());
	for (auto i = state.begin(); i != state.end(); ++i) {
		const char fieldType = i->second;
		if (!isPackage(fieldType)) {
			Log::trace(LOG_MAP, "deadlock fieldType %i ignored", fieldType);
			continue;
		}
		int col;
		int row;
		const int index = i->first;
		if (!state.getColRowFromIndex(index, col, row)) {
			Log::error(LOG_MAP, "could not get col and row from index %i", index);
			continue;
		}
		if (!canMovePackage(s, col, row)) {
			Log::info(LOG_MAP, "Found a deadlock at %i:%i", col, row);
			deadlocks[index] = Sokoban::DEADLOCK;
			continue;
		}
		Log::debug(LOG_MAP, "package at %i:%i can still be moved", col, row);
	}

	return deadlocks;
}

bool DeadlockDetector::hasDeadlock(const BoardState& state) {
	Log::debug(LOG_MAP, "check whether there is a deadlock");
	DeadlockState s;
	s.state = state;
	const StateMap& deadlockMap = calculateDeadlockFields(s);
	return !deadlockMap.empty();
}

bool DeadlockDetector::canMovePackage(DeadlockState& state, int col, int row) {
	// we only need to check two directions here
	const bool left = canMovePackageLeft(state, col, row);
	const bool up = canMovePackageUp(state, col, row);
	const bool moveable = left || up;
	if (moveable)
		Log::trace(LOG_MAP, "package on %i:%i can be moved", col, row);
	else
		Log::trace(LOG_MAP, "package on %i:%i can't be moved", col, row);
	return moveable;
}

bool DeadlockDetector::canMovePackageDirection(DeadlockState& state, char dir, int col, int row) {
	const int index = state.state.getIndex(col, row);
	auto i = state.checkedState.find(index);
	if (i != state.checkedState.end())
		return i->second;
	int lx, ly;
	getXY(dir, lx, ly);
	int ox, oy;
	getOppositeXY(dir, ox, oy);
	if (!state.state.isFree(col + lx, row + ly)) {
		if (state.state.isPackage(col + lx, row + ly)) {
			const bool m = canMovePackage(state, col + lx, row + ly);
			state.checkedState.insert(std::make_pair(index, m));
			return m;
		}
		return false;
	}
	if (!state.state.isFree(col + ox, row + oy)) {
		if (state.state.isPackage(col + ox, row + oy)) {
			const bool m = canMovePackage(state, col + ox, row + oy);
			state.checkedState.insert(std::make_pair(index, m));
			return m;
		}
		return false;
	}

	return true;
}

bool DeadlockDetector::canMovePackageLeft(DeadlockState& state, int col, int row) {
	return canMovePackageDirection(state, MOVE_LEFT, col, row);
}

bool DeadlockDetector::canMovePackageRight(DeadlockState& state, int col, int row) {
	return canMovePackageDirection(state, MOVE_RIGHT, col, row);
}

bool DeadlockDetector::canMovePackageUp(DeadlockState& state, int col, int row) {
	return canMovePackageDirection(state, MOVE_UP, col, row);
}

bool DeadlockDetector::canMovePackageDown(DeadlockState& state, int col, int row) {
	return canMovePackageDirection(state, MOVE_DOWN, col, row);
}

}
