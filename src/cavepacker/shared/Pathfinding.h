#pragma once

#include "BoardState.h"
#include <vector>

namespace cavepacker {

extern bool astar(const BoardState& bstate, int startIndex, int endIndex, std::vector<int>& path);

}
