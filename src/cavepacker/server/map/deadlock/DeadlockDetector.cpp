#include "DeadlockDetector.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/server/map/SokobanTiles.h"
#include "cavepacker/server/map/BoardState.h"

namespace cavepacker {

void DeadlockDetector::clear() {
	_simple.clear();
	_frozen.clear();
}

void DeadlockDetector::init(const BoardState& state) {
	_simple.init(state);
	_frozen.init(state);
}

bool DeadlockDetector::hasDeadlock(const BoardState& state) {
	if (_simple.hasDeadlock(state))
		return true;

	if (_frozen.hasDeadlock(_simple, state))
		return true;

	return false;
}

DeadlockSet DeadlockDetector::getDeadlocks() {
	DeadlockSet s;
	_simple.fillDeadlocks(s);
	_frozen.fillDeadlocks(s);
	return s;
}

}
