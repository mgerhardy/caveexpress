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

uint16_t bit (WfcMapGenerator::Cell c)
{
	return static_cast<uint16_t>(1u << static_cast<unsigned>(c));
}

const uint16_t MASK_AIR = bit(WfcMapGenerator::Cell::Air);
const uint16_t MASK_ROCK = bit(WfcMapGenerator::Cell::Rock);
const uint16_t MASK_GROUND = bit(WfcMapGenerator::Cell::Ground);
const uint16_t MASK_LEDGEL = bit(WfcMapGenerator::Cell::LedgeL);
const uint16_t MASK_LEDGER = bit(WfcMapGenerator::Cell::LedgeR);
const uint16_t MASK_UL = bit(WfcMapGenerator::Cell::UndercutL);
const uint16_t MASK_UR = bit(WfcMapGenerator::Cell::UndercutR);
const uint16_t MASK_SHIM = bit(WfcMapGenerator::Cell::Shim);
const uint16_t MASK_SURFACE = MASK_GROUND | MASK_LEDGEL | MASK_LEDGER;
const uint16_t MASK_FILL = MASK_AIR | MASK_ROCK;
const uint16_t MASK_ALL = static_cast<uint16_t>((1u << static_cast<unsigned>(WfcMapGenerator::Cell::Count)) - 1u);

bool containsId (const std::string& id, const char* needle)
{
	return id.find(needle) != std::string::npos;
}
}

WfcMapGenerator::WfcMapGenerator (const ThemeType& theme, unsigned int width, unsigned int height, unsigned int caveTarget) :
		_theme(&theme), _width(std::max(10u, width)), _height(std::max(8u, height)), _caveTarget(std::max(1u, caveTarget))
{
	_waterHeight = std::min(2.5f, std::max(1.0f, static_cast<float>(_height) * 0.18f));
	collectSprites();
}

bool WfcMapGenerator::isSurface (Cell c) const
{
	return c == Cell::Ground || c == Cell::LedgeL || c == Cell::LedgeR;
}

bool WfcMapGenerator::isShimSprite (const SpriteDefPtr& def) const
{
	return containsId(def->id, "shim");
}

bool WfcMapGenerator::isUndercutLeftSprite (const SpriteDefPtr& def) const
{
	// slope-*-left-02: bottom-right open / solid SW — used under left platform mass edges
	return containsId(def->id, "slope") && containsId(def->id, "left-02");
}

bool WfcMapGenerator::isUndercutRightSprite (const SpriteDefPtr& def) const
{
	return containsId(def->id, "slope") && containsId(def->id, "right-02");
}

bool WfcMapGenerator::isFullRockSprite (const SpriteDefPtr& def) const
{
	if (!SpriteTypes::isRock(def->type))
		return false;
	if (def->width > 1.01f || def->height > 1.01f)
		return false;
	if (isShimSprite(def) || isUndercutLeftSprite(def) || isUndercutRightSprite(def))
		return false;
	// Face rocks / partial solids (left-04, right-04, …) have custom polygons — skip for fill
	if (def->hasShape())
		return false;
	return true;
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
		if (def->width > 1.01f || def->height > 1.01f)
			continue;

		if (isShimSprite(def))
			_shims.push_back(def);
		else if (isUndercutLeftSprite(def))
			_undercutL.push_back(def);
		else if (isUndercutRightSprite(def))
			_undercutR.push_back(def);
		else if (isFullRockSprite(def))
			_rocksFull.push_back(def);
		else if (SpriteTypes::isGround(type))
			_grounds.push_back(def);
		else if (SpriteTypes::isGroundLeft(type))
			_groundLeft.push_back(def);
		else if (SpriteTypes::isGroundRight(type))
			_groundRight.push_back(def);
		else if (SpriteTypes::isBackground(type)) {
			if (containsId(def->id, "cave-art"))
				_caveArt.push_back(def);
			else
				_backgrounds.push_back(def);
		} else if (SpriteTypes::isCave(type))
			_caves.push_back(def);
		else if (SpriteTypes::isWindow(type))
			_windows.push_back(def);
	}
	if (_grounds.empty())
		_grounds = _rocksFull;
	if (_groundLeft.empty())
		_groundLeft = _grounds;
	if (_groundRight.empty())
		_groundRight = _grounds;
	if (_undercutL.empty())
		_undercutL = _rocksFull;
	if (_undercutR.empty())
		_undercutR = _rocksFull;
	if (_shims.empty())
		_shims = _rocksFull;
	if (_backgrounds.empty() && !_caveArt.empty())
		_backgrounds = _caveArt;
	Log::info(LOG_GAMEIMPL, "WFC sprites rock=%i ground=%i ledgeL=%i ledgeR=%i underL=%i underR=%i shim=%i bg=%i cave=%i",
			(int)_rocksFull.size(), (int)_grounds.size(), (int)_groundLeft.size(), (int)_groundRight.size(),
			(int)_undercutL.size(), (int)_undercutR.size(), (int)_shims.size(), (int)_backgrounds.size(),
			(int)_caves.size());
}

bool WfcMapGenerator::compatible (Cell a, Cell b, int dir) const
{
	// dir: 0=N 1=E 2=S 3=W — sockets from sprites.lua shapes + rock-*/ice-* map grammar
	auto surface = [this] (Cell c) { return isSurface(c); };
	auto solid = [] (Cell c) {
		return c == Cell::Rock || c == Cell::UndercutL || c == Cell::UndercutR || c == Cell::Shim;
	};

	switch (a) {
	case Cell::Air:
		if (b == Cell::Air)
			return true;
		if (solid(b))
			return true; // cliffs / walls
		if (surface(b))
			return dir == 2 || dir == 1 || dir == 3; // surface below or beside air
		break;

	case Cell::Rock:
		if (b == Cell::Rock || b == Cell::Air || b == Cell::Shim)
			return true;
		if (b == Cell::UndercutL || b == Cell::UndercutR)
			return true;
		if (surface(b))
			return dir == 0 || dir == 1 || dir == 3; // surface above/beside rock mass
		break;

	case Cell::Ground:
		if (dir == 0)
			return b == Cell::Air; // walkable top needs open sky/façade
		if (dir == 2)
			// Solid decks sit on rock; floating village-style segments may hang over air
			return solid(b) || b == Cell::Ground || b == Cell::Air;
		// sides: continue platform, meet ledge ends, or butt into a wall
		return b == Cell::Ground || b == Cell::LedgeL || b == Cell::LedgeR || b == Cell::Rock || b == Cell::Air;

	case Cell::LedgeL:
		// left ledge: open air to the west, platform continues east (ground/ledgeR)
		if (dir == 3)
			return b == Cell::Air;
		if (dir == 1)
			return b == Cell::Ground || b == Cell::LedgeR || b == Cell::LedgeL;
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return b == Cell::Air || solid(b); // floating or supported
		break;

	case Cell::LedgeR:
		if (dir == 1)
			return b == Cell::Air;
		if (dir == 3)
			return b == Cell::Ground || b == Cell::LedgeL || b == Cell::LedgeR;
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return b == Cell::Air || solid(b);
		break;

	case Cell::UndercutL:
		// slope-left-02 under mass: often rock/surface above, air below, rock toward mass (east)
		if (dir == 0)
			return surface(b) || b == Cell::Rock;
		if (dir == 2)
			return b == Cell::Air;
		if (dir == 1)
			return b == Cell::Rock || b == Cell::UndercutL || surface(b);
		if (dir == 3)
			return b == Cell::Air || b == Cell::Rock;
		break;

	case Cell::UndercutR:
		if (dir == 0)
			return surface(b) || b == Cell::Rock;
		if (dir == 2)
			return b == Cell::Air;
		if (dir == 3)
			return b == Cell::Rock || b == Cell::UndercutR || surface(b);
		if (dir == 1)
			return b == Cell::Air || b == Cell::Rock;
		break;

	case Cell::Shim:
		// tip hanging under rock into air
		if (dir == 0)
			return b == Cell::Rock;
		if (dir == 2)
			return b == Cell::Air;
		return b == Cell::Air || b == Cell::Rock;

	default:
		break;
	}
	return false;
}

int WfcMapGenerator::entropy (uint16_t mask) const
{
	int n = 0;
	for (unsigned i = 0; i < static_cast<unsigned>(Cell::Count); ++i)
		if (mask & (1u << i))
			++n;
	return n;
}

WfcMapGenerator::Cell WfcMapGenerator::observe (uint16_t mask, unsigned int& rng) const
{
	struct Opt { Cell c; int w; };
	Opt opts[8];
	int count = 0;
	auto add = [&] (Cell c, int w) {
		if (mask & bit(c)) {
			opts[count].c = c;
			opts[count].w = w;
			++count;
		}
	};
	// Prefer air/rock for free cells; surfaces/undercuts mainly arrive via seeds/decorate
	add(Cell::Air, 60);
	add(Cell::Rock, 40);
	add(Cell::Ground, 8);
	add(Cell::LedgeL, 3);
	add(Cell::LedgeR, 3);
	add(Cell::UndercutL, 4);
	add(Cell::UndercutR, 4);
	add(Cell::Shim, 2);
	if (count == 0)
		return Cell::Air;
	int total = 0;
	for (int i = 0; i < count; ++i)
		total += opts[i].w;
	int roll = randRange(rng, total);
	for (int i = 0; i < count; ++i) {
		roll -= opts[i].w;
		if (roll < 0)
			return opts[i].c;
	}
	return opts[count - 1].c;
}

void WfcMapGenerator::applyBorderSeeds (std::vector<uint16_t>& domain) const
{
	const int waterRows = std::max(1, static_cast<int>(std::ceil(_waterHeight)));
	for (unsigned y = 0; y < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			uint16_t& cell = domain[x + y * _width];
			if (y < 2)
				cell = MASK_AIR;
			else if (y >= _height - static_cast<unsigned>(waterRows))
				cell = MASK_ROCK;
			else if (x == 0 || x == _width - 1)
				// Side walls may open into air so platform ledges can terminate cleanly
				cell = MASK_AIR | MASK_ROCK;
			else
				// Free cells collapse to air/rock; surfaces come from platform seeds
				cell = MASK_FILL;
		}
	}
}

void WfcMapGenerator::seedPlatforms (std::vector<uint16_t>& domain, unsigned int& rng) const
{
	const int waterRows = std::max(1, static_cast<int>(std::ceil(_waterHeight)));
	const int minY = 3;
	const int maxY = static_cast<int>(_height) - waterRows - 2;
	if (maxY <= minY)
		return;

	const int bandCount = 2 + randRange(rng, 2); // 2–3 platform bands
	std::vector<int> rows;
	int guard = 0;
	while (static_cast<int>(rows.size()) < bandCount && guard++ < 40) {
		const int y = minY + randRange(rng, maxY - minY + 1);
		bool ok = true;
		for (int existing : rows) {
			if (std::abs(existing - y) < 5) {
				ok = false;
				break;
			}
		}
		if (ok)
			rows.push_back(y);
	}
	std::sort(rows.begin(), rows.end());

	for (int y : rows) {
		int x = 2 + randRange(rng, 2);
		while (x < static_cast<int>(_width) - 3) {
			const int len = 3 + randRange(rng, 3); // 3–5
			if (x + len >= static_cast<int>(_width) - 2)
				break;
			const bool floating = randRange(rng, 5) == 0;
			const int fillDepth = floating ? 0 : (1 + randRange(rng, 2)); // 1–2 rock under decks

			// Ensure open air beside ledge ends (matches ground-left/right physics sockets)
			if (x - 1 >= 0)
				domain[(x - 1) + y * _width] = MASK_AIR;
			if (x + len < static_cast<int>(_width))
				domain[(x + len) + y * _width] = MASK_AIR;

			for (int i = 0; i < len; ++i) {
				const int cx = x + i;
				Cell surface = Cell::Ground;
				if (i == 0)
					surface = Cell::LedgeL;
				else if (i == len - 1)
					surface = Cell::LedgeR;
				domain[cx + y * _width] = bit(surface);

				for (int d = 1; d <= fillDepth; ++d) {
					const int cy = y + d;
					if (cy >= static_cast<int>(_height) - waterRows)
						break;
					domain[cx + cy * _width] = MASK_ROCK;
				}
			}
			x += len + 2 + randRange(rng, 3); // gap 2–4
		}
	}
}

bool WfcMapGenerator::propagate (std::vector<uint16_t>& domain, int startX, int startY) const
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
		const uint16_t mask = domain[idx];
		if (mask == 0)
			return false;

		for (int d = 0; d < 4; ++d) {
			const int nx = cx + DIR[d][0];
			const int ny = cy + DIR[d][1];
			if (nx < 0 || ny < 0 || nx >= static_cast<int>(_width) || ny >= static_cast<int>(_height))
				continue;
			const int nidx = nx + ny * static_cast<int>(_width);
			uint16_t& nmask = domain[nidx];
			uint16_t allowed = 0;
			for (unsigned a = 0; a < static_cast<unsigned>(Cell::Count); ++a) {
				if ((mask & (1u << a)) == 0)
					continue;
				for (unsigned b = 0; b < static_cast<unsigned>(Cell::Count); ++b) {
					if ((nmask & (1u << b)) == 0)
						continue;
					if (compatible(static_cast<Cell>(a), static_cast<Cell>(b), d))
						allowed = static_cast<uint16_t>(allowed | (1u << b));
				}
			}
			const uint16_t narrowed = static_cast<uint16_t>(nmask & allowed);
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

void WfcMapGenerator::decorateEdges (std::vector<Cell>& grid, unsigned int& rng) const
{
	std::vector<Cell> next = grid;
	for (unsigned y = 1; y + 1 < _height; ++y) {
		for (unsigned x = 1; x + 1 < _width; ++x) {
			const Cell c = grid[x + y * _width];
			const Cell above = at(grid, static_cast<int>(x), static_cast<int>(y) - 1, Cell::Air);
			const Cell below = at(grid, static_cast<int>(x), static_cast<int>(y) + 1, Cell::Air);
			const Cell left = at(grid, static_cast<int>(x) - 1, static_cast<int>(y), Cell::Air);
			const Cell right = at(grid, static_cast<int>(x) + 1, static_cast<int>(y), Cell::Air);

			if (c != Cell::Rock)
				continue;

			// Undercut under surface edge into open air (slope-*-02)
			if (isSurface(above) && below == Cell::Air) {
				if (left == Cell::Air || at(grid, static_cast<int>(x) - 1, static_cast<int>(y) + 1, Cell::Air) == Cell::Air)
					next[x + y * _width] = Cell::UndercutL;
				else if (right == Cell::Air || at(grid, static_cast<int>(x) + 1, static_cast<int>(y) + 1, Cell::Air) == Cell::Air)
					next[x + y * _width] = Cell::UndercutR;
			}

			// Shim tip: rock above, air below, open sides
			if (above == Cell::Rock && below == Cell::Air && (left == Cell::Air || right == Cell::Air) && randRange(rng, 3) == 0)
				next[x + y * _width] = Cell::Shim;
		}
	}
	grid.swap(next);
}

bool WfcMapGenerator::collapse (std::vector<uint16_t>& domain, std::vector<Cell>& out, unsigned int& rng) const
{
	out.assign(_width * _height, Cell::Air);
	applyBorderSeeds(domain);
	seedPlatforms(domain, rng);

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
			break;

		const Cell chosen = observe(domain[bestIdx], rng);
		domain[bestIdx] = bit(chosen);
		if (!propagate(domain, bestIdx % static_cast<int>(_width), bestIdx / static_cast<int>(_width)))
			return false;
	}

	for (unsigned i = 0; i < domain.size(); ++i) {
		if (domain[i] == 0)
			return false;
		Cell resolved = Cell::Air;
		// Prefer more specific types when multiple bits remain
		static const Cell preference[] = {
			Cell::LedgeL, Cell::LedgeR, Cell::Ground, Cell::UndercutL, Cell::UndercutR,
			Cell::Shim, Cell::Rock, Cell::Air
		};
		for (Cell c : preference) {
			if (domain[i] & bit(c)) {
				resolved = c;
				break;
			}
		}
		out[i] = resolved;
	}

	decorateEdges(out, rng);
	return true;
}

bool WfcMapGenerator::isAirConnected (const std::vector<Cell>& grid) const
{
	int start = -1;
	int airCount = 0;
	for (unsigned i = 0; i < grid.size(); ++i) {
		if (grid[i] == Cell::Air) {
			++airCount;
			if (start < 0)
				start = static_cast<int>(i);
		}
	}
	if (start < 0 || airCount == 0)
		return false;

	std::vector<uint8_t> visited(grid.size(), 0);
	std::queue<int> q;
	q.push(start);
	visited[start] = 1;
	int reached = 0;
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
	return reached * 100 >= airCount * 80;
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

void WfcMapGenerator::setTile (Result& result, int x, int y, const SpriteDefPtr& def) const
{
	if (!def)
		return;
	for (MapTileDefinition& tile : result.tiles) {
		if (static_cast<int>(tile.x) == x && static_cast<int>(tile.y) == y) {
			tile.spriteDef = def;
			return;
		}
	}
	result.tiles.emplace_back(static_cast<gridCoord>(x), static_cast<gridCoord>(y), def, 0);
}

void WfcMapGenerator::instantiate (const std::vector<Cell>& grid, Result& result, unsigned int& rng) const
{
	result.tiles.clear();
	result.caves.clear();
	result.emitters.clear();
	result.startPositions.clear();

	std::vector<std::pair<int, int>> surfaceCells;
	std::vector<std::pair<int, int>> airCells;

	for (unsigned y = 0; y < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			const Cell cell = grid[x + y * _width];
			SpriteDefPtr def;
			switch (cell) {
			case Cell::Rock:
				def = pick(_rocksFull, rng);
				break;
			case Cell::Ground:
				def = pick(_grounds, rng);
				surfaceCells.push_back({ static_cast<int>(x), static_cast<int>(y) });
				break;
			case Cell::LedgeL:
				def = pick(_groundLeft, rng);
				surfaceCells.push_back({ static_cast<int>(x), static_cast<int>(y) });
				break;
			case Cell::LedgeR:
				def = pick(_groundRight, rng);
				surfaceCells.push_back({ static_cast<int>(x), static_cast<int>(y) });
				break;
			case Cell::UndercutL:
				def = pick(_undercutL, rng);
				break;
			case Cell::UndercutR:
				def = pick(_undercutR, rng);
				break;
			case Cell::Shim:
				def = pick(_shims, rng);
				break;
			case Cell::Air:
			default: {
				const bool art = !_caveArt.empty() && randRange(rng, 6) == 0;
				def = art ? pick(_caveArt, rng) : pick(_backgrounds, rng);
				airCells.push_back({ static_cast<int>(x), static_cast<int>(y) });
				break;
			}
			}
			if (def)
				result.tiles.emplace_back(static_cast<gridCoord>(x), static_cast<gridCoord>(y), def, 0);
		}
	}

	// Caves on façades above surfaces, with adjacent windows (matches rock-*/ice-* maps)
	for (int i = static_cast<int>(surfaceCells.size()) - 1; i > 0; --i) {
		const int j = randRange(rng, i + 1);
		std::swap(surfaceCells[i], surfaceCells[j]);
	}
	unsigned cavesPlaced = 0;
	for (const auto& g : surfaceCells) {
		if (cavesPlaced >= _caveTarget)
			break;
		const int cx = g.first;
		const int cy = g.second - 1;
		if (cy < 1 || at(grid, cx, cy) != Cell::Air)
			continue;
		const SpriteDefPtr cave = pick(_caves, rng);
		if (!cave)
			break;
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

		// Window beside cave on the same façade row
		if (!_windows.empty()) {
			const int wx = (randRange(rng, 2) == 0) ? cx + 1 : cx - 1;
			if (wx > 0 && wx < static_cast<int>(_width) - 1 && at(grid, wx, cy) == Cell::Air
					&& isSurface(at(grid, wx, cy + 1))) {
				setTile(result, wx, cy, pick(_windows, rng));
			}
		}
	}

	std::vector<std::pair<int, int>> starts;
	for (const auto& a : airCells) {
		if (a.second < 1)
			continue;
		if (a.second >= static_cast<int>(_height) - static_cast<int>(std::ceil(_waterHeight)) - 1)
			continue;
		starts.push_back(a);
	}
	if (!starts.empty()) {
		const auto& s = starts[randRange(rng, static_cast<int>(starts.size()))];
		result.startPositions.push_back({ string::toString(s.first), string::toString(s.second) });
	}

	auto trySurfaceEmitter = [&] (const EntityType& type, int chance) {
		for (const auto& g : surfaceCells) {
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
	trySurfaceEmitter(EntityTypes::STONE, 5);
	trySurfaceEmitter(EntityTypes::TREE, 6);
	trySurfaceEmitter(EntityTypes::NPC_WALKING, 8);
	const EntityType& packageTarget = ThemeTypes::isIce(*_theme) ? EntityTypes::PACKAGETARGET_ICE : EntityTypes::PACKAGETARGET_ROCK;
	if (trySurfaceEmitter(packageTarget, 3)) {
		const EntityType& packageType = ThemeTypes::isIce(*_theme) ? EntityTypes::PACKAGE_ICE : EntityTypes::PACKAGE_ROCK;
		trySurfaceEmitter(packageType, 2);
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

	if (_rocksFull.empty() || _backgrounds.empty()) {
		Log::error(LOG_GAMEIMPL, "WFC: missing theme sprites");
		return result;
	}

	unsigned int rng = seed ? seed : 1u;
	for (int attempt = 0; attempt < 32; ++attempt) {
		std::vector<uint16_t> domain(_width * _height, MASK_ALL);
		std::vector<Cell> grid;
		unsigned int attemptSeed = rng + static_cast<unsigned int>(attempt) * 9973u;
		if (!collapse(domain, grid, attemptSeed))
			continue;
		if (!isAirConnected(grid))
			continue;

		int surfaces = 0;
		for (Cell c : grid)
			if (isSurface(c))
				++surfaces;
		if (surfaces < 6)
			continue;

		instantiate(grid, result, attemptSeed);
		if (result.startPositions.empty())
			continue;
		if (result.caves.empty() && !_caves.empty())
			continue;

		result.success = true;
		Log::info(LOG_GAMEIMPL, "WFC generated %ux%u map (attempt %i, surfaces=%i caves=%i)", _width, _height,
				attempt + 1, surfaces, (int)result.caves.size());
		return result;
	}

	Log::error(LOG_GAMEIMPL, "WFC failed to produce a valid map");
	return result;
}

}
