#include "WfcMapGenerator.h"
#include "common/EmitterDefinition.h"
#include "common/MapSettings.h"
#include "common/Log.h"
#include "common/String.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include <algorithm>
#include <queue>
#include <cmath>

namespace caveexpress {

namespace {
const int DIR[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } }; // N E S W
const int OPP[4] = { 2, 3, 0, 1 };

unsigned int nextRng (unsigned int& state)
{
	state = state * 1664525u + 1013904223u;
	return state;
}

int randRange (unsigned int& state, int maxExclusive)
{
	if (maxExclusive <= 0)
		return 0;
	return static_cast<int>(nextRng(state) % static_cast<unsigned int>(maxExclusive));
}

uint8_t bit (WfcMapGenerator::Cell c)
{
	return static_cast<uint8_t>(1u << static_cast<unsigned>(c));
}

const uint8_t ALL = static_cast<uint8_t>((1u << static_cast<unsigned>(WfcMapGenerator::Cell::Count)) - 1u);
}

WfcMapGenerator::WfcMapGenerator (const ThemeType& theme, unsigned int width, unsigned int height, unsigned int caveTarget) :
		_theme(&theme), _width(std::max(8u, width)), _height(std::max(6u, height)), _caveTarget(std::max(1u, caveTarget))
{
	_waterHeight = std::min(2.5f, static_cast<float>(_height) * 0.2f);
	collectSprites();
}

void WfcMapGenerator::collectSprites ()
{
	for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
		const SpriteDefPtr& def = i->second;
		if (!def->theme.isNone() && def->theme != *_theme)
			continue;
		const SpriteType& type = def->type;
		if (!SpriteTypes::isMapTile(type))
			continue;
		// Prefer 1x1 tiles for WFC instantiation
		if (def->width > 1.01f || def->height > 1.01f)
			continue;
		if (SpriteTypes::isRock(type))
			_rocks.push_back(def);
		else if (SpriteTypes::isGround(type))
			_grounds.push_back(def);
		else if (SpriteTypes::isGroundLeft(type))
			_groundLeft.push_back(def);
		else if (SpriteTypes::isGroundRight(type))
			_groundRight.push_back(def);
		else if (SpriteTypes::isBackground(type))
			_backgrounds.push_back(def);
		else if (SpriteTypes::isCave(type))
			_caves.push_back(def);
		else if (SpriteTypes::isWindow(type))
			_windows.push_back(def);
	}
	if (_grounds.empty())
		_grounds = _rocks;
	if (_groundLeft.empty())
		_groundLeft = _grounds;
	if (_groundRight.empty())
		_groundRight = _grounds;
	Log::info(LOG_GAMEIMPL, "WFC sprites rock=%i ground=%i bg=%i cave=%i",
			(int)_rocks.size(), (int)_grounds.size(), (int)_backgrounds.size(), (int)_caves.size());
}

bool WfcMapGenerator::compatible (Cell a, Cell b, int dir) const
{
	// dir = direction from a toward neighbor b (0=N,1=E,2=S,3=W)
	// Ground is a 1-cell-thick walkable platform: Air above, Solid below,
	// and Air/Ground/Solid allowed on the sides (platform edges / walls).
	switch (a) {
	case Cell::Air:
		if (b == Cell::Air || b == Cell::Solid)
			return true;
		if (b == Cell::Ground)
			return dir == 2 || dir == 1 || dir == 3; // below or beside
		break;
	case Cell::Solid:
		if (b == Cell::Solid || b == Cell::Air)
			return true;
		if (b == Cell::Ground)
			return dir == 0 || dir == 1 || dir == 3; // above or beside
		break;
	case Cell::Ground:
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return b == Cell::Solid;
		return b == Cell::Ground || b == Cell::Solid || b == Cell::Air;
	default:
		break;
	}
	return false;
}

int WfcMapGenerator::entropy (uint8_t mask) const
{
	int n = 0;
	for (unsigned i = 0; i < static_cast<unsigned>(Cell::Count); ++i)
		if (mask & (1u << i))
			++n;
	return n;
}

WfcMapGenerator::Cell WfcMapGenerator::observe (uint8_t mask, unsigned int& rng) const
{
	int options[3];
	int weights[3];
	int count = 0;
	if (mask & bit(Cell::Air)) {
		options[count] = static_cast<int>(Cell::Air);
		weights[count++] = 55;
	}
	if (mask & bit(Cell::Solid)) {
		options[count] = static_cast<int>(Cell::Solid);
		weights[count++] = 35;
	}
	if (mask & bit(Cell::Ground)) {
		options[count] = static_cast<int>(Cell::Ground);
		weights[count++] = 20;
	}
	if (count == 0)
		return Cell::Air;
	int total = 0;
	for (int i = 0; i < count; ++i)
		total += weights[i];
	int roll = randRange(rng, total);
	for (int i = 0; i < count; ++i) {
		roll -= weights[i];
		if (roll < 0)
			return static_cast<Cell>(options[i]);
	}
	return static_cast<Cell>(options[count - 1]);
}

void WfcMapGenerator::applySeeds (std::vector<uint8_t>& domain) const
{
	const int waterRows = std::max(1, static_cast<int>(std::ceil(_waterHeight)));
	for (unsigned y = 0; y < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			uint8_t& cell = domain[x + y * _width];
			// sky band
			if (y < 2)
				cell = bit(Cell::Air);
			// water bed / floor
			else if (y >= _height - static_cast<unsigned>(waterRows))
				cell = bit(Cell::Solid);
			// side borders — solid walls with occasional air gaps mid-map
			else if (x == 0 || x == _width - 1) {
				if (y < _height / 3)
					cell = bit(Cell::Air) | bit(Cell::Solid);
				else
					cell = bit(Cell::Solid);
			} else if (y == 2) {
				// near ceiling: air or rare solid overhang
				cell = bit(Cell::Air) | bit(Cell::Solid);
			} else {
				cell = ALL;
			}
		}
	}
}

bool WfcMapGenerator::propagate (std::vector<uint8_t>& domain, int startX, int startY) const
{
	std::queue<int> q;
	q.push(startX + startY * static_cast<int>(_width));
	std::vector<uint8_t> seen(domain.size(), 0);
	seen[startX + startY * _width] = 1;

	while (!q.empty()) {
		const int idx = q.front();
		q.pop();
		const int cx = idx % static_cast<int>(_width);
		const int cy = idx / static_cast<int>(_width);
		const uint8_t mask = domain[idx];
		if (mask == 0)
			return false;

		for (int d = 0; d < 4; ++d) {
			const int nx = cx + DIR[d][0];
			const int ny = cy + DIR[d][1];
			if (nx < 0 || ny < 0 || nx >= static_cast<int>(_width) || ny >= static_cast<int>(_height))
				continue;
			const int nidx = nx + ny * static_cast<int>(_width);
			uint8_t& nmask = domain[nidx];
			uint8_t allowed = 0;
			for (unsigned a = 0; a < static_cast<unsigned>(Cell::Count); ++a) {
				if ((mask & (1u << a)) == 0)
					continue;
				for (unsigned b = 0; b < static_cast<unsigned>(Cell::Count); ++b) {
					if ((nmask & (1u << b)) == 0)
						continue;
					if (compatible(static_cast<Cell>(a), static_cast<Cell>(b), d))
						allowed |= static_cast<uint8_t>(1u << b);
				}
			}
			const uint8_t narrowed = nmask & allowed;
			if (narrowed == nmask)
				continue;
			if (narrowed == 0)
				return false;
			nmask = narrowed;
			if (!seen[nidx]) {
				seen[nidx] = 1;
				q.push(nidx);
			}
		}
	}
	return true;
}

bool WfcMapGenerator::collapse (std::vector<uint8_t>& domain, std::vector<Cell>& out, unsigned int& rng) const
{
	out.assign(_width * _height, Cell::Air);
	applySeeds(domain);

	// initial propagation from seeded cells
	for (unsigned y = 0; y < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			if (entropy(domain[x + y * _width]) == 1) {
				if (!propagate(domain, static_cast<int>(x), static_cast<int>(y)))
					return false;
			}
		}
	}

	for (;;) {
		int bestIdx = -1;
		int bestEntropy = 100;
		int ties = 0;
		for (unsigned i = 0; i < domain.size(); ++i) {
			const int e = entropy(domain[i]);
			if (e <= 1)
				continue;
			if (e < bestEntropy) {
				bestEntropy = e;
				bestIdx = static_cast<int>(i);
				ties = 1;
			} else if (e == bestEntropy) {
				++ties;
				if (randRange(rng, ties) == 0)
					bestIdx = static_cast<int>(i);
			}
		}
		if (bestIdx < 0)
			break; // fully collapsed

		const Cell chosen = observe(domain[bestIdx], rng);
		domain[bestIdx] = bit(chosen);
		const int x = bestIdx % static_cast<int>(_width);
		const int y = bestIdx / static_cast<int>(_width);
		if (!propagate(domain, x, y))
			return false;
	}

	for (unsigned i = 0; i < domain.size(); ++i) {
		if (domain[i] == 0)
			return false;
		if (domain[i] & bit(Cell::Ground))
			out[i] = Cell::Ground;
		else if (domain[i] & bit(Cell::Solid))
			out[i] = Cell::Solid;
		else
			out[i] = Cell::Air;
	}
	return true;
}

bool WfcMapGenerator::isAirConnected (const std::vector<Cell>& grid) const
{
	int start = -1;
	for (unsigned i = 0; i < grid.size(); ++i) {
		if (grid[i] == Cell::Air) {
			start = static_cast<int>(i);
			break;
		}
	}
	if (start < 0)
		return false;

	std::vector<uint8_t> visited(grid.size(), 0);
	std::queue<int> q;
	q.push(start);
	visited[start] = 1;
	int airCount = 0;
	int reached = 0;
	for (Cell c : grid)
		if (c == Cell::Air)
			++airCount;

	while (!q.empty()) {
		const int idx = q.front();
		q.pop();
		++reached;
		const int x = idx % static_cast<int>(_width);
		const int y = idx / static_cast<int>(_width);
		for (int d = 0; d < 4; ++d) {
			const int nx = x + DIR[d][0];
			const int ny = y + DIR[d][1];
			if (nx < 0 || ny < 0 || nx >= static_cast<int>(_width) || ny >= static_cast<int>(_height))
				continue;
			const int nidx = nx + ny * static_cast<int>(_width);
			if (visited[nidx] || grid[nidx] != Cell::Air)
				continue;
			visited[nidx] = 1;
			q.push(nidx);
		}
	}
	// Allow a little fragmentation (pockets) but require most air reachable
	return reached * 100 >= airCount * 85;
}

WfcMapGenerator::Cell WfcMapGenerator::at (const std::vector<Cell>& grid, int x, int y, Cell fallback) const
{
	if (x < 0 || y < 0 || x >= static_cast<int>(_width) || y >= static_cast<int>(_height))
		return fallback;
	return grid[x + y * _width];
}

SpriteDefPtr WfcMapGenerator::pick (const std::vector<SpriteDefPtr>& list, unsigned int& rng) const
{
	if (list.empty())
		return SpriteDefPtr();
	return list[randRange(rng, static_cast<int>(list.size()))];
}

void WfcMapGenerator::instantiate (const std::vector<Cell>& grid, Result& result, unsigned int& rng) const
{
	result.tiles.clear();
	result.caves.clear();
	result.emitters.clear();
	result.startPositions.clear();

	std::vector<std::pair<int, int>> groundCells;
	std::vector<std::pair<int, int>> airCells;

	for (unsigned y = 0; y < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			const Cell cell = grid[x + y * _width];
			const Cell left = at(grid, static_cast<int>(x) - 1, static_cast<int>(y));
			const Cell right = at(grid, static_cast<int>(x) + 1, static_cast<int>(y));
			const Cell above = at(grid, static_cast<int>(x), static_cast<int>(y) - 1, Cell::Air);
			const Cell below = at(grid, static_cast<int>(x), static_cast<int>(y) + 1);

			SpriteDefPtr def;
			if (cell == Cell::Solid) {
				def = pick(_rocks, rng);
			} else if (cell == Cell::Ground) {
				if (left == Cell::Air && !_groundLeft.empty())
					def = pick(_groundLeft, rng);
				else if (right == Cell::Air && !_groundRight.empty())
					def = pick(_groundRight, rng);
				else
					def = pick(_grounds, rng);
				groundCells.push_back({ static_cast<int>(x), static_cast<int>(y) });
			} else {
				// Air → background; occasional windows when framed by solid
				if (!_windows.empty() && above == Cell::Solid && below == Cell::Ground && randRange(rng, 8) == 0)
					def = pick(_windows, rng);
				else
					def = pick(_backgrounds, rng);
				airCells.push_back({ static_cast<int>(x), static_cast<int>(y) });
			}
			if (def)
				result.tiles.emplace_back(static_cast<gridCoord>(x), static_cast<gridCoord>(y), def, 0);
		}
	}

	// Place caves on air cells directly above ground
	unsigned cavesPlaced = 0;
	for (int i = static_cast<int>(groundCells.size()) - 1; i > 0; --i) {
		const int j = randRange(rng, i + 1);
		std::swap(groundCells[i], groundCells[j]);
	}
	for (const auto& g : groundCells) {
		if (cavesPlaced >= _caveTarget)
			break;
		const int cx = g.first;
		const int cy = g.second - 1;
		if (cy < 1)
			continue;
		if (at(grid, cx, cy) != Cell::Air)
			continue;
		// Prefer niches: solid on at least one side
		const bool niche = at(grid, cx - 1, cy) == Cell::Solid || at(grid, cx + 1, cy) == Cell::Solid;
		if (!niche && randRange(rng, 3) != 0)
			continue;
		const SpriteDefPtr cave = pick(_caves, rng);
		if (!cave)
			break;
		// Caves are separate from map tiles (see RandomMapContext::rndAddCave)
		result.tiles.erase(std::remove_if(result.tiles.begin(), result.tiles.end(),
				[cx, cy] (const MapTileDefinition& t) {
					return static_cast<int>(t.x) == cx && static_cast<int>(t.y) == cy;
				}), result.tiles.end());
		const EntityType* npc = &EntityType::NONE;
		const int npcRoll = randRange(rng, 4);
		if (npcRoll == 0)
			npc = &EntityTypes::NPC_FRIENDLY_MAN;
		else if (npcRoll == 1)
			npc = &EntityTypes::NPC_FRIENDLY_WOMAN;
		else if (npcRoll == 2)
			npc = &EntityTypes::NPC_FRIENDLY_GRANDPA;
		result.caves.emplace_back(static_cast<gridCoord>(cx), static_cast<gridCoord>(cy), cave, *npc, 5000);
		++cavesPlaced;
	}

	// Player start: air cell not too close to water, preferably above ground somewhere
	std::vector<std::pair<int, int>> starts;
	for (const auto& a : airCells) {
		if (a.second >= static_cast<int>(_height) - static_cast<int>(_waterHeight) - 1)
			continue;
		if (a.second < 1)
			continue;
		starts.push_back(a);
	}
	if (!starts.empty()) {
		const auto& s = starts[randRange(rng, static_cast<int>(starts.size()))];
		result.startPositions.push_back({ string::toString(s.first), string::toString(s.second) });
	}

	// Emitters on ground platforms (mirrors RandomMapContext priorities)
	auto tryGroundEmitter = [&] (const EntityType& type, int chance) {
		for (const auto& g : groundCells) {
			if (randRange(rng, chance) != 0)
				continue;
			const int px = g.first;
			const int py = g.second - 1;
			if (at(grid, px, py) != Cell::Air)
				continue;
			result.emitters.emplace_back(static_cast<gridCoord>(px), static_cast<gridCoord>(py), type, 1, 0, "");
			return true;
		}
		return false;
	};
	tryGroundEmitter(EntityTypes::STONE, 5);
	tryGroundEmitter(EntityTypes::TREE, 6);
	tryGroundEmitter(EntityTypes::NPC_WALKING, 8);
	const EntityType& packageTarget = ThemeTypes::isIce(*_theme) ? EntityTypes::PACKAGETARGET_ICE : EntityTypes::PACKAGETARGET_ROCK;
	if (tryGroundEmitter(packageTarget, 3)) {
		const EntityType& packageType = ThemeTypes::isIce(*_theme) ? EntityTypes::PACKAGE_ICE : EntityTypes::PACKAGE_ROCK;
		tryGroundEmitter(packageType, 2);
	}
}

WfcMapGenerator::Result WfcMapGenerator::generate (unsigned int seed)
{
	Result result;
	result.title = "WFC " + _theme->name;
	result.settings[msn::WIDTH] = string::toString(_width);
	result.settings[msn::HEIGHT] = string::toString(_height);
	result.settings[msn::THEME] = _theme->name;
	result.settings[msn::WATER_HEIGHT] = string::toString(_waterHeight);
	result.settings[msn::WATER_CHANGE] = "0.0";
	result.settings[msn::GRAVITY] = string::toString(msdv::GRAVITY);
	result.settings[msn::POINTS] = string::toString(msdv::POINTS);
	result.settings[msn::REFERENCETIME] = string::toString(msdv::REFERENCETIME);
	result.settings[msn::PACKAGE_TRANSFER_COUNT] = "3";
	result.settings[msn::FLYING_NPC] = "false";
	result.settings[msn::FISH_NPC] = "false";
	result.settings[msn::WIND] = "0.0";

	if (_rocks.empty() || _backgrounds.empty()) {
		Log::error(LOG_GAMEIMPL, "WFC: missing theme sprites");
		return result;
	}

	unsigned int rng = seed ? seed : 1u;
	for (int attempt = 0; attempt < 24; ++attempt) {
		std::vector<uint8_t> domain(_width * _height, ALL);
		std::vector<Cell> grid;
		unsigned int attemptSeed = rng + static_cast<unsigned int>(attempt) * 9973u;
		if (!collapse(domain, grid, attemptSeed))
			continue;
		if (!isAirConnected(grid))
			continue;

		instantiate(grid, result, attemptSeed);
		if (result.startPositions.empty())
			continue;
		if (result.caves.empty() && !_caves.empty())
			continue;

		result.success = true;
		Log::info(LOG_GAMEIMPL, "WFC generated %ux%u map (attempt %i, caves=%i)", _width, _height, attempt + 1,
				(int)result.caves.size());
		return result;
	}

	Log::error(LOG_GAMEIMPL, "WFC failed to produce a valid map");
	return result;
}

}
