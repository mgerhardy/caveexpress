#include "DeadlockDetector.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/server/map/SokobanTiles.h"

namespace cavepacker {

StateMap DeadlockDetector::calculateDeadlockFields(const BoardState& state) {
	StateMap deadlocks;

	for (auto i = state.begin(); i != state.end(); ++i) {
		const int index = i->second;
		if (!isPackage(index)) {
			continue;
		}
		int col;
		int row;
		if (!state.getColRowFromIndex(index, col, row)) {
			continue;
		}
		if (!state.canMoveLeft(col, row) && !state.canMoveRight(col, row) &&
				!state.canMoveUp(col, row) && !state.canMoveDown(col, row)) {
			deadlocks[index] = Sokoban::DEADLOCK;
			continue;
		}
	}

	return deadlocks;
}

bool DeadlockDetector::hasDeadlock(const BoardState& state) {
	const StateMap& deadlockMap = calculateDeadlockFields(state);
	return !deadlockMap.empty();
}

}
