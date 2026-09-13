#pragma once

#include "common/Math.h"

namespace player {

/** Matches MAX_CLIENTS. Kept here to avoid a network include cycle. */
const uint8_t COLOR_COUNT = 4;
const uint8_t COLOR_NONE = 255;

inline bool isValidIndex (uint8_t index)
{
	return index < COLOR_COUNT;
}

template<typename PlayerList>
inline void markUsedColorIndices (bool used[COLOR_COUNT], const PlayerList& list)
{
	for (const auto* p : list) {
		if (isValidIndex(p->getColorIndex()))
			used[p->getColorIndex()] = true;
	}
}

/** Lowest free slot among spawned and waiting players (not spectators). */
template<typename PlayerList>
inline uint8_t allocateColorIndex (const PlayerList& spawned, const PlayerList& waiting)
{
	bool used[COLOR_COUNT] = {};
	markUsedColorIndices(used, spawned);
	markUsedColorIndices(used, waiting);
	for (uint8_t i = 0; i < COLOR_COUNT; ++i) {
		if (!used[i])
			return i;
	}
	return COLOR_NONE;
}

template<typename PlayerList>
inline uint8_t colorIndexForJoin (bool multiplayer, bool spectator, const PlayerList& spawned,
		const PlayerList& waiting)
{
	if (!multiplayer || spectator)
		return COLOR_NONE;
	return allocateColorIndex(spawned, waiting);
}

/** Saturated tints so clothes/skin stay visible (not a primary-only multiply). */
inline const Color& color (uint8_t index)
{
	static const Color palette[COLOR_COUNT] = {
		{ 1.00f, 0.35f, 0.35f, 1.0f },
		{ 0.35f, 0.55f, 1.00f, 1.0f },
		{ 0.35f, 1.00f, 0.40f, 1.0f },
		{ 1.00f, 0.85f, 0.25f, 1.0f },
	};
	if (!isValidIndex(index))
		return colorWhite;
	return palette[index];
}

}
