#include "ClosedDiagonalDetector.h"

namespace cavepacker {

void ClosedDiagonalDetector::clear() {
}

void ClosedDiagonalDetector::init(const BoardState& s) {
}

bool ClosedDiagonalDetector::hasDeadlock(uint32_t millisStart, const BoardState& s) {
	return false;
}

void ClosedDiagonalDetector::fillDeadlocks(DeadlockSet& set) const {
}

}
