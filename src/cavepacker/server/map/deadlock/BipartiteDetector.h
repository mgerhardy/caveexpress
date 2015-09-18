#pragma once

#include "DeadlockTypes.h"

namespace cavepacker {

class BoardState;

class BipartiteDetector {
public:
	void clear();
	void init(const BoardState& s);
	bool hasDeadlock(uint32_t millisStart, const BoardState& s);
	void fillDeadlocks(DeadlockSet& set) const;
};

}
