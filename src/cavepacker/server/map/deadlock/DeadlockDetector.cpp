#include "DeadlockDetector.h"
#include "DeadlockTypes.h"
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

bool DeadlockDetector::hasDeadlock(const BoardState& state, uint32_t timeout) {
	_start = SDL_GetTicks();
	uint32_t maxMillis = timeout;
	const uint32_t timeoutMillis = _start + maxMillis;

	const uint32_t simpleMaxMillis = 5u;
	if (_simple.hasDeadlock(_start, simpleMaxMillis, state))
		return true;

	TIMEOUTREACHED(timeoutMillis)

	const uint32_t frozenMaxMillis = 35u;
	if (_frozen.hasDeadlock(_start, frozenMaxMillis, _simple, state))
		return true;

	TIMEOUTREACHED(timeoutMillis)

	if (_bipartite.hasDeadlock(_start, state))
		return true;

	TIMEOUTREACHED(timeoutMillis)

	if (_closedDiagonal.hasDeadlock(_start, state))
		return true;

	TIMEOUTREACHED(timeoutMillis)

	if (_corral.hasDeadlock(_start, state))
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
