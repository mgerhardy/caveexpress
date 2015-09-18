#include "CorralDetector.h"

namespace cavepacker {

void CorralDetector::clear() {
}

void CorralDetector::init(const BoardState& s) {
}

bool CorralDetector::hasDeadlock(uint32_t millisStart, const BoardState& s) {
	return false;
}

void CorralDetector::fillDeadlocks(DeadlockSet& set) const {
}

}
