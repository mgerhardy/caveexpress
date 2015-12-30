#include "BipartiteDetector.h"

namespace cavepacker {

void BipartiteDetector::clear() {
}

void BipartiteDetector::init(const BoardState& s) {
}

bool BipartiteDetector::hasDeadlock(uint32_t millisStart, uint32_t millisTimeout, const BoardState& s) {
	return false;
}

void BipartiteDetector::fillDeadlocks(DeadlockSet& set) const {
}

}
