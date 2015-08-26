#include "DeadlockDetector.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/server/map/SokobanTiles.h"
#include "cavepacker/server/map/BoardState.h"

namespace cavepacker {

void DeadlockDetector::clear() {
	_simple.clear();
}

void DeadlockDetector::init(const BoardState& state) {
	_simple.init(state);
}

bool DeadlockDetector::hasDeadlock(const BoardState& state) {
	return _simple.hasDeadlock(state);
}

DeadlockSet DeadlockDetector::getDeadlocks() {
	DeadlockSet s;
	_simple.fillDeadlocks(s);
	return s;
}

}
