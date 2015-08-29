#include "DeadlockDetector.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/server/map/SokobanTiles.h"
#include "cavepacker/server/map/BoardState.h"

namespace cavepacker {

void DeadlockDetector::clear() {
	_simple.clear();
	_frozen.clear();
	_bipartite.clear();
	_closedDiagonal.clear();
	_corral.clear();
}

void DeadlockDetector::init(const BoardState& state) {
	_simple.init(state);
	_frozen.init(state);
	_bipartite.init(state);
	_closedDiagonal.init(state);
	_corral.init(state);
}

bool DeadlockDetector::hasDeadlock(const BoardState& state) {
	// TODO: limit the time that these tests may consume
	if (_simple.hasDeadlock(state))
		return true;

	if (_frozen.hasDeadlock(_simple, state))
		return true;

	if (_bipartite.hasDeadlock(state))
		return true;

	if (_closedDiagonal.hasDeadlock(state))
		return true;

	if (_corral.hasDeadlock(state))
		return true;

	return false;
}

DeadlockSet DeadlockDetector::getDeadlocks() {
	DeadlockSet s;
	_simple.fillDeadlocks(s);
	_frozen.fillDeadlocks(s);
	_bipartite.fillDeadlocks(s);
	_closedDiagonal.fillDeadlocks(s);
	_corral.fillDeadlocks(s);
	return s;
}

}
