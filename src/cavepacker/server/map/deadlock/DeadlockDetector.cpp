#include "DeadlockDetector.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/server/map/SokobanTiles.h"

namespace cavepacker {

StateMap DeadlockDetector::calculateDeadlockFields(const BoardState& state) {
	StateMap deadlocks;

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
		if (!state.canMove(col, row)) {
			Log::info(LOG_MAP, "Found a deadlock at %i:%i", col, row);
			deadlocks[index] = Sokoban::DEADLOCK;
			continue;
		}
		Log::debug(LOG_MAP, "package at %i:%i can still be moved", col, row);
	}

	return deadlocks;
}

bool DeadlockDetector::hasDeadlock(const BoardState& state) {
	Log::info(LOG_MAP, "check whether there is a deadlock");
	const StateMap& deadlockMap = calculateDeadlockFields(state);
	return !deadlockMap.empty();
}

}
