#pragma once

#include <stdint.h>
#include <vector>

namespace spectate {

/**
 * Next index in a circular list. dir > 0 goes forward, dir < 0 backward.
 * If current is out of range, start at the first (forward) or last (backward) entry.
 */
inline int wrapIndex (int current, int count, int dir)
{
	if (count <= 0)
		return -1;
	if (dir == 0)
		dir = 1;
	if (current < 0 || current >= count)
		return dir > 0 ? 0 : count - 1;
	int next = current + (dir > 0 ? 1 : -1);
	if (next >= count)
		return 0;
	if (next < 0)
		return count - 1;
	return next;
}

/** Cycle a stable list of entity ids. Empty list returns 0. */
inline uint16_t cycleId (const std::vector<uint16_t>& ids, uint16_t current, int dir)
{
	if (ids.empty())
		return 0;
	int idx = -1;
	for (size_t i = 0; i < ids.size(); ++i) {
		if (ids[i] == current) {
			idx = static_cast<int>(i);
			break;
		}
	}
	const int next = wrapIndex(idx, static_cast<int>(ids.size()), dir);
	if (next < 0)
		return 0;
	return ids[static_cast<size_t>(next)];
}

}
