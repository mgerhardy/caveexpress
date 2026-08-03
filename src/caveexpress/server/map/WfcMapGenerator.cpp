#include "WfcMapGenerator.h"
#include "common/EmitterDefinition.h"
#include "common/MapSettings.h"
#include "common/Log.h"
#include "common/String.h"
#include "common/LUALibrary.h"
#include "common/ThemeType.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include <algorithm>
#include <queue>
#include <cmath>
#include <set>

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
const uint16_t MASK_FILL = MASK_AIR | MASK_ROCK;
const uint16_t MASK_ALL = static_cast<uint16_t>((1u << static_cast<unsigned>(WfcMapGenerator::Cell::Count)) - 1u);

bool containsId (const std::string& id, const char* needle)
{
	return id.find(needle) != std::string::npos;
}

int keyXY (int x, int y)
{
	return (y << 16) | (x & 0xffff);
}
}

WfcRules WfcRules::loadFromLua (const std::string& path)
{
	WfcRules r;
	LUA lua(false);
	if (!lua.load(path)) {
		Log::info(LOG_GAMEIMPL, "WFC: no %s, using defaults", path.c_str());
		return r;
	}
	if (!lua.execute("getWfcRules", 1)) {
		Log::info(LOG_GAMEIMPL, "WFC: getWfcRules() missing in %s, using defaults", path.c_str());
		return r;
	}
	lua_State* L = lua.getState();
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return r;
	}
	auto getTable = [&] (const char* name) -> bool {
		lua_getfield(L, -1, name);
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			return false;
		}
		return true;
	};
	auto pop = [&] () { lua_pop(L, 1); };

	if (getTable("caves")) {
		r.caveTarget = lua.getValueIntegerFromTable("target", r.caveTarget);
		r.minCaveSeparation = lua.getValueIntegerFromTable("minSeparation", r.minCaveSeparation);
		r.caveNpcChance = lua.getValueFloatFromTable("npcChance", r.caveNpcChance);
		pop();
	}
	if (getTable("windows")) {
		r.windowsEnabled = lua.getValueBoolFromTable("enabled", r.windowsEnabled);
		r.oneWindowPerCave = lua.getValueBoolFromTable("onePerCave", r.oneWindowPerCave);
		r.forbidAdjacentWindows = lua.getValueBoolFromTable("forbidAdjacentWindows", r.forbidAdjacentWindows);
		pop();
	}
	if (getTable("platforms")) {
		r.platformBandMin = lua.getValueIntegerFromTable("bandMin", r.platformBandMin);
		r.platformBandMax = lua.getValueIntegerFromTable("bandMax", r.platformBandMax);
		r.minVerticalGap = lua.getValueIntegerFromTable("minVerticalGap", r.minVerticalGap);
		r.lengthMin = lua.getValueIntegerFromTable("lengthMin", r.lengthMin);
		r.lengthMax = lua.getValueIntegerFromTable("lengthMax", r.lengthMax);
		r.gapMin = lua.getValueIntegerFromTable("gapMin", r.gapMin);
		r.gapMax = lua.getValueIntegerFromTable("gapMax", r.gapMax);
		r.floatingChance = lua.getValueFloatFromTable("floatingChance", r.floatingChance);
		pop();
	}
	if (getTable("bridges")) {
		r.bridgesEnabled = lua.getValueBoolFromTable("enabled", r.bridgesEnabled);
		r.bridgeMaxGap = lua.getValueIntegerFromTable("maxGap", r.bridgeMaxGap);
		pop();
	}
	if (getTable("weights")) {
		r.weightAir = lua.getValueIntegerFromTable("air", r.weightAir);
		r.weightRock = lua.getValueIntegerFromTable("rock", r.weightRock);
		r.weightGround = lua.getValueIntegerFromTable("ground", r.weightGround);
		r.weightLedge = lua.getValueIntegerFromTable("ledge", r.weightLedge);
		r.weightUndercut = lua.getValueIntegerFromTable("undercut", r.weightUndercut);
		r.weightShim = lua.getValueIntegerFromTable("shim", r.weightShim);
		pop();
	}
	if (getTable("packageTarget")) {
		r.packageTargetRequired = lua.getValueBoolFromTable("required", r.packageTargetRequired);
		r.packageTargetSidesWalkable = lua.getValueBoolFromTable("sidesMustBeWalkable", r.packageTargetSidesWalkable);
		r.packageTargetRequireAirAbove = lua.getValueBoolFromTable("requireAirOppositeSupport", r.packageTargetRequireAirAbove);
		pop();
	}
	if (getTable("emitters")) {
		r.stoneChance = lua.getValueIntegerFromTable("stoneChance", r.stoneChance);
		r.treeChance = lua.getValueIntegerFromTable("treeChance", r.treeChance);
		r.walkingChance = lua.getValueIntegerFromTable("walkingChance", r.walkingChance);
		r.packageChance = lua.getValueIntegerFromTable("packageChance", r.packageChance);
		pop();
	}
	if (getTable("decor")) {
		r.caveArtChance = lua.getValueIntegerFromTable("caveArtChance", r.caveArtChance);
		pop();
	}
	lua_pop(L, 1);
	return r;
}

WfcMapGenerator::WfcMapGenerator (const ThemeType& theme, unsigned int width, unsigned int height,
		unsigned int caveTarget, const WfcRules& rules) :
		_theme(&theme), _width(std::max(10u, width)), _height(std::max(8u, height)),
		_caveTarget(std::max(1u, caveTarget)), _rules(rules)
{
	_waterHeight = std::min(2.5f, std::max(1.0f, static_cast<float>(_height) * 0.18f));
	collectSprites();
}

bool WfcMapGenerator::isWalkableSurface (Cell c) const
{
	return c == Cell::Ground || c == Cell::LedgeL || c == Cell::LedgeR || c == Cell::Bridge;
}

bool WfcMapGenerator::isColliderSolid (Cell c) const
{
	return c == Cell::Rock || c == Cell::UndercutL || c == Cell::UndercutR || c == Cell::Shim
			|| isWalkableSurface(c);
}

bool WfcMapGenerator::isShimSprite (const SpriteDefPtr& def) const
{
	return containsId(def->id, "shim");
}

bool WfcMapGenerator::isHangingGroundSprite (const SpriteDefPtr& def) const
{
	// tile-ground-05/06 are thin liane-like decks - collision only on the top slab; need air beneath.
	return containsId(def->id, "ground-05") || containsId(def->id, "ground-06");
}

bool WfcMapGenerator::isUndercutLeftSprite (const SpriteDefPtr& def) const
{
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
		if (SpriteTypes::isBridgeLeft(type) && def->width <= 1.01f)
			_bridgeLeft.push_back(def);
		else if (SpriteTypes::isBridgeRight(type) && def->width <= 1.01f)
			_bridgeRight.push_back(def);
		else if (SpriteTypes::isBridgePlank(type) && def->width <= 1.01f)
			_bridgePlank.push_back(def);
		else if (SpriteTypes::isPackageTarget(type) && def->isStatic())
			_packageTargets.push_back(def);

		if (!SpriteTypes::isMapTile(type) && !SpriteTypes::isBridge(type) && !SpriteTypes::isPackageTarget(type))
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
		else if (SpriteTypes::isGround(type)) {
			if (isHangingGroundSprite(def))
				_groundsHanging.push_back(def);
			else
				_grounds.push_back(def);
		} else if (SpriteTypes::isGroundLeft(type))
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
	if (_bridgePlank.empty())
		_bridgePlank = _bridgeLeft;
	Log::info(LOG_GAMEIMPL, "WFC sprites rock=%i ground=%i bridge=%i/%i/%i pkgTarget=%i cave=%i",
			(int)_rocksFull.size(), (int)_grounds.size(), (int)_bridgeLeft.size(), (int)_bridgePlank.size(),
			(int)_bridgeRight.size(), (int)_packageTargets.size(), (int)_caves.size());
}

bool WfcMapGenerator::compatible (Cell a, Cell b, int dir) const
{
	auto walkable = [this] (Cell c) { return isWalkableSurface(c); };
	auto solid = [this] (Cell c) { return isColliderSolid(c); };

	switch (a) {
	case Cell::Air:
		if (b == Cell::Air)
			return true;
		if (solid(b))
			return true;
		break;

	case Cell::Rock:
		if (b == Cell::Bridge)
			return false; // bridges only neighbor ground/bridge
		if (b == Cell::Rock || b == Cell::Air || b == Cell::Shim)
			return true;
		if (b == Cell::UndercutL || b == Cell::UndercutR)
			return true;
		if (walkable(b))
			return dir == 0 || dir == 1 || dir == 3;
		break;

	case Cell::Ground:
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return solid(b) || b == Cell::Ground || b == Cell::Bridge || b == Cell::Air;
		return b == Cell::Ground || b == Cell::Bridge || b == Cell::LedgeL || b == Cell::LedgeR
				|| b == Cell::Rock || b == Cell::Air;

	case Cell::Bridge:
		// Bridges sit over open air (background on the same cell) and only connect to ground/bridge.
		if (dir == 0 || dir == 2)
			return b == Cell::Air;
		return b == Cell::Ground || b == Cell::Bridge || b == Cell::LedgeL || b == Cell::LedgeR;

	case Cell::LedgeL:
		if (dir == 3)
			return b == Cell::Air;
		if (dir == 1)
			return b == Cell::Ground || b == Cell::Bridge || b == Cell::LedgeR || b == Cell::LedgeL;
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return b == Cell::Air; // overhang - never solid fill under ledge sprites
		break;

	case Cell::LedgeR:
		if (dir == 1)
			return b == Cell::Air;
		if (dir == 3)
			return b == Cell::Ground || b == Cell::Bridge || b == Cell::LedgeL || b == Cell::LedgeR;
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return b == Cell::Air;
		break;

	case Cell::UndercutL:
		if (dir == 0)
			return walkable(b) || b == Cell::Rock;
		if (dir == 2)
			return b == Cell::Air;
		if (dir == 1)
			return b == Cell::Rock || b == Cell::UndercutL || walkable(b);
		if (dir == 3)
			return b == Cell::Air || b == Cell::Rock;
		break;

	case Cell::UndercutR:
		if (dir == 0)
			return walkable(b) || b == Cell::Rock;
		if (dir == 2)
			return b == Cell::Air;
		if (dir == 3)
			return b == Cell::Rock || b == Cell::UndercutR || walkable(b);
		if (dir == 1)
			return b == Cell::Air || b == Cell::Rock;
		break;

	case Cell::Shim:
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
	Opt opts[16];
	int count = 0;
	auto add = [&] (Cell c, int w) {
		if (mask & bit(c)) {
			opts[count].c = c;
			opts[count].w = w;
			++count;
		}
	};
	add(Cell::Air, _rules.weightAir);
	add(Cell::Rock, _rules.weightRock);
	add(Cell::Ground, _rules.weightGround);
	add(Cell::LedgeL, _rules.weightLedge);
	add(Cell::LedgeR, _rules.weightLedge);
	add(Cell::UndercutL, _rules.weightUndercut);
	add(Cell::UndercutR, _rules.weightUndercut);
	add(Cell::Shim, _rules.weightShim);
	add(Cell::Bridge, 1);
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
				cell = MASK_AIR | MASK_ROCK;
			else
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

	const int bandSpan = std::max(0, _rules.platformBandMax - _rules.platformBandMin);
	const int bandCount = _rules.platformBandMin + randRange(rng, bandSpan + 1);
	std::vector<int> rows;
	int guard = 0;
	while (static_cast<int>(rows.size()) < bandCount && guard++ < 40) {
		const int y = minY + randRange(rng, maxY - minY + 1);
		bool ok = true;
		for (int existing : rows) {
			if (std::abs(existing - y) < _rules.minVerticalGap) {
				ok = false;
				break;
			}
		}
		if (ok)
			rows.push_back(y);
	}
	std::sort(rows.begin(), rows.end());

	const int lenSpan = std::max(0, _rules.lengthMax - _rules.lengthMin);
	const int gapSpan = std::max(0, _rules.gapMax - _rules.gapMin);
	for (int y : rows) {
		int x = 2 + randRange(rng, 2);
		while (x < static_cast<int>(_width) - 3) {
			const int len = _rules.lengthMin + randRange(rng, lenSpan + 1);
			if (x + len >= static_cast<int>(_width) - 2)
				break;
			const bool floating = randRange(rng, 1000) < static_cast<int>(_rules.floatingChance * 1000.0f);
			const int fillDepth = floating ? 0 : (1 + randRange(rng, 2));

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

				// Ledge end sprites overhang - keep air beneath them.
				if (surface == Cell::LedgeL || surface == Cell::LedgeR) {
					const int cy = y + 1;
					if (cy < static_cast<int>(_height) - waterRows)
						domain[cx + cy * _width] = MASK_AIR;
					continue;
				}

				for (int d = 1; d <= fillDepth; ++d) {
					const int cy = y + d;
					if (cy >= static_cast<int>(_height) - waterRows)
						break;
					domain[cx + cy * _width] = MASK_ROCK;
				}
			}
			x += len + _rules.gapMin + randRange(rng, gapSpan + 1);
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

			if (isWalkableSurface(above) && below == Cell::Air) {
				if (left == Cell::Air || at(grid, static_cast<int>(x) - 1, static_cast<int>(y) + 1, Cell::Air) == Cell::Air)
					next[x + y * _width] = Cell::UndercutL;
				else if (right == Cell::Air || at(grid, static_cast<int>(x) + 1, static_cast<int>(y) + 1, Cell::Air) == Cell::Air)
					next[x + y * _width] = Cell::UndercutR;
			}

			if (above == Cell::Rock && below == Cell::Air && (left == Cell::Air || right == Cell::Air) && randRange(rng, 3) == 0)
				next[x + y * _width] = Cell::Shim;
		}
	}
	grid.swap(next);
}

void WfcMapGenerator::ensureWalkableTops (std::vector<Cell>& grid) const
{
	for (unsigned y = 1; y < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			const Cell c = grid[x + y * _width];
			const Cell above = at(grid, static_cast<int>(x), static_cast<int>(y) - 1, Cell::Air);
			if (above != Cell::Air)
				continue;
			if (c == Cell::Rock || c == Cell::UndercutL || c == Cell::UndercutR || c == Cell::Shim)
				grid[x + y * _width] = Cell::Ground;
		}
	}
	// Ground ledge tiles must overhang open air.
	for (unsigned y = 0; y + 1 < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			const Cell c = grid[x + y * _width];
			if (c != Cell::LedgeL && c != Cell::LedgeR)
				continue;
			grid[x + (y + 1) * _width] = Cell::Air;
		}
	}
}

void WfcMapGenerator::placeBridgeSpans (std::vector<Cell>& grid, unsigned int& rng) const
{
	if (!_rules.bridgesEnabled)
		return;
	for (unsigned y = 2; y + 1 < _height; ++y) {
		int x = 1;
		while (x < static_cast<int>(_width) - 1) {
			if (!isWalkableSurface(at(grid, x, static_cast<int>(y))) || at(grid, x + 1, static_cast<int>(y)) != Cell::Air) {
				++x;
				continue;
			}
			int gapStart = x + 1;
			int gapEnd = gapStart;
			while (gapEnd < static_cast<int>(_width) - 1 && at(grid, gapEnd, static_cast<int>(y)) == Cell::Air)
				++gapEnd;
			const int gapLen = gapEnd - gapStart;
			if (gapLen >= 1 && gapLen <= _rules.bridgeMaxGap) {
				const Cell leftAnchor = at(grid, x, static_cast<int>(y));
				const Cell rightAnchor = at(grid, gapEnd, static_cast<int>(y));
				const bool leftOk = leftAnchor == Cell::Ground || leftAnchor == Cell::LedgeL || leftAnchor == Cell::LedgeR;
				const bool rightOk = rightAnchor == Cell::Ground || rightAnchor == Cell::LedgeL || rightAnchor == Cell::LedgeR;
				bool openBelow = leftOk && rightOk;
				for (int bx = gapStart; bx < gapEnd && openBelow; ++bx) {
					if (at(grid, bx, static_cast<int>(y) + 1) != Cell::Air)
						openBelow = false;
				}
				if (openBelow && randRange(rng, 2) == 0) {
					for (int bx = gapStart; bx < gapEnd; ++bx)
						grid[bx + y * _width] = Cell::Bridge;
				}
			}
			x = std::max(x + 1, gapEnd);
		}
	}
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
		static const Cell preference[] = {
			Cell::LedgeL, Cell::LedgeR, Cell::Ground, Cell::Bridge, Cell::UndercutL, Cell::UndercutR,
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
	ensureWalkableTops(out);
	placeBridgeSpans(out, rng);
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
	if (airCount == 0 || start < 0)
		return false;

	std::vector<uint8_t> seen(grid.size(), 0);
	std::queue<int> q;
	q.push(start);
	seen[start] = 1;
	int reached = 0;
	while (!q.empty()) {
		const int idx = q.front();
		q.pop();
		++reached;
		const int cx = idx % static_cast<int>(_width);
		const int cy = idx / static_cast<int>(_width);
		for (int d = 0; d < 4; ++d) {
			const int nx = cx + DIR[d][0];
			const int ny = cy + DIR[d][1];
			if (nx < 0 || ny < 0 || nx >= static_cast<int>(_width) || ny >= static_cast<int>(_height))
				continue;
			const int nidx = nx + ny * static_cast<int>(_width);
			if (seen[nidx] || grid[nidx] != Cell::Air)
				continue;
			seen[nidx] = 1;
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
	std::set<int> occupied;

	for (unsigned y = 0; y < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			const Cell cell = grid[x + y * _width];
			SpriteDefPtr def;
			switch (cell) {
			case Cell::Rock:
				def = pick(_rocksFull, rng);
				break;
			case Cell::Ground: {
				// Hanging decks (ground-05/06) only when air is beneath; otherwise solid ground.
				const Cell below = at(grid, static_cast<int>(x), static_cast<int>(y) + 1);
				if (below == Cell::Air && !_groundsHanging.empty() && randRange(rng, 4) == 0)
					def = pick(_groundsHanging, rng);
				else
					def = pick(_grounds, rng);
				surfaceCells.emplace_back(static_cast<int>(x), static_cast<int>(y));
				break;
			}
			case Cell::LedgeL:
				def = pick(_groundLeft, rng);
				surfaceCells.emplace_back(static_cast<int>(x), static_cast<int>(y));
				break;
			case Cell::LedgeR:
				def = pick(_groundRight, rng);
				surfaceCells.emplace_back(static_cast<int>(x), static_cast<int>(y));
				break;
			case Cell::Bridge: {
				const Cell left = at(grid, static_cast<int>(x) - 1, static_cast<int>(y));
				const Cell right = at(grid, static_cast<int>(x) + 1, static_cast<int>(y));
				const bool leftWalk = left == Cell::Ground || left == Cell::LedgeL || left == Cell::LedgeR || left == Cell::Bridge;
				const bool rightWalk = right == Cell::Ground || right == Cell::LedgeL || right == Cell::LedgeR || right == Cell::Bridge;
				if (!leftWalk && rightWalk && !_bridgeLeft.empty())
					def = pick(_bridgeLeft, rng);
				else if (leftWalk && !rightWalk && !_bridgeRight.empty())
					def = pick(_bridgeRight, rng);
				else
					def = pick(_bridgePlank.empty() ? _grounds : _bridgePlank, rng);
				// Same-cell background under the bridge (matches hand maps / editor rule)
				const SpriteDefPtr bg = pick(_backgrounds, rng);
				if (bg)
					result.tiles.emplace_back(static_cast<gridCoord>(x), static_cast<gridCoord>(y), bg, 0);
				surfaceCells.emplace_back(static_cast<int>(x), static_cast<int>(y));
				break;
			}
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
				const bool art = !_caveArt.empty() && _rules.caveArtChance > 0
						&& randRange(rng, _rules.caveArtChance) == 0;
				def = art ? pick(_caveArt, rng) : pick(_backgrounds, rng);
				airCells.emplace_back(static_cast<int>(x), static_cast<int>(y));
				break;
			}
			}
			if (def)
				result.tiles.emplace_back(static_cast<gridCoord>(x), static_cast<gridCoord>(y), def, 0);
		}
	}

	for (int i = static_cast<int>(surfaceCells.size()) - 1; i > 0; --i) {
		const int j = randRange(rng, i + 1);
		std::swap(surfaceCells[i], surfaceCells[j]);
	}

	unsigned cavesPlaced = 0;
	std::vector<std::pair<int, int>> placedCaves;
	for (const auto& g : surfaceCells) {
		if (cavesPlaced >= _caveTarget)
			break;
		const int cx = g.first;
		const int cy = g.second - 1;
		if (cy < 1 || at(grid, cx, cy) != Cell::Air)
			continue;
		if (occupied.count(keyXY(cx, cy)))
			continue;

		bool farEnough = true;
		for (const auto& prev : placedCaves) {
			const int d = std::max(std::abs(prev.first - cx), std::abs(prev.second - cy));
			if (d < _rules.minCaveSeparation) {
				farEnough = false;
				break;
			}
		}
		if (!farEnough)
			continue;

		const SpriteDefPtr cave = pick(_caves, rng);
		if (!cave)
			break;
		result.tiles.erase(std::remove_if(result.tiles.begin(), result.tiles.end(),
				[cx, cy] (const MapTileDefinition& t) {
					return static_cast<int>(t.x) == cx && static_cast<int>(t.y) == cy;
				}), result.tiles.end());

		const EntityType* npc = &EntityType::NONE;
		if (randRange(rng, 1000) < static_cast<int>(_rules.caveNpcChance * 1000.0f)) {
			const int npcRoll = randRange(rng, 3);
			if (npcRoll == 0)
				npc = &EntityTypes::NPC_FRIENDLY_MAN;
			else if (npcRoll == 1)
				npc = &EntityTypes::NPC_FRIENDLY_WOMAN;
			else
				npc = &EntityTypes::NPC_FRIENDLY_GRANDPA;
		}
		result.caves.emplace_back(static_cast<gridCoord>(cx), static_cast<gridCoord>(cy), cave, *npc, 5000);
		occupied.insert(keyXY(cx, cy));
		placedCaves.emplace_back(cx, cy);
		++cavesPlaced;

		if (_rules.windowsEnabled && !_windows.empty()) {
			const int side = randRange(rng, 2) == 0 ? -1 : 1;
			const int wx = cx + side;
			if (wx > 0 && wx < static_cast<int>(_width) - 1 && at(grid, wx, cy) == Cell::Air
					&& isWalkableSurface(at(grid, wx, cy + 1)) && !occupied.count(keyXY(wx, cy))) {
				auto hasWindowAt = [&] (int x, int y) {
					for (const MapTileDefinition& t : result.tiles) {
						if (static_cast<int>(t.x) == x && static_cast<int>(t.y) == y && t.spriteDef
								&& SpriteTypes::isWindow(t.spriteDef->type))
							return true;
					}
					return false;
				};
				if (!(_rules.forbidAdjacentWindows && (hasWindowAt(wx - 1, cy) || hasWindowAt(wx + 1, cy)))) {
					setTile(result, wx, cy, pick(_windows, rng));
					occupied.insert(keyXY(wx, cy));
				}
			}
		}
	}

	// Package target as map tile in niche (angle 0)
	bool placedTarget = false;
	if (!_packageTargets.empty()) {
		for (const auto& g : surfaceCells) {
			const int x = g.first;
			const int y = g.second;
			if (occupied.count(keyXY(x, y)))
				continue;
			const Cell left = at(grid, x - 1, y);
			const Cell right = at(grid, x + 1, y);
			const Cell below = at(grid, x, y + 1);
			const Cell above = at(grid, x, y - 1);
			if (_rules.packageTargetRequireAirAbove && above != Cell::Air)
				continue;
			if (!isColliderSolid(below))
				continue;
			if (_rules.packageTargetSidesWalkable) {
				if (!isWalkableSurface(left) || !isWalkableSurface(right))
					continue;
			} else if (!isColliderSolid(left) || !isColliderSolid(right)) {
				continue;
			}
			setTile(result, x, y, pick(_packageTargets, rng));
			occupied.insert(keyXY(x, y));
			placedTarget = true;
			break;
		}
	}
	if (_rules.packageTargetRequired && !placedTarget && !_packageTargets.empty()) {
		// Relaxed: solid sides instead of walkable
		for (const auto& g : surfaceCells) {
			const int x = g.first;
			const int y = g.second;
			if (occupied.count(keyXY(x, y)))
				continue;
			if (at(grid, x, y - 1) != Cell::Air)
				continue;
			if (!isColliderSolid(at(grid, x, y + 1)))
				continue;
			if (!isColliderSolid(at(grid, x - 1, y)) || !isColliderSolid(at(grid, x + 1, y)))
				continue;
			setTile(result, x, y, pick(_packageTargets, rng));
			occupied.insert(keyXY(x, y));
			placedTarget = true;
			break;
		}
	}

	std::vector<std::pair<int, int>> starts;
	for (const auto& a : airCells) {
		if (a.second < 1)
			continue;
		if (a.second >= static_cast<int>(_height) - static_cast<int>(std::ceil(_waterHeight)) - 1)
			continue;
		if (occupied.count(keyXY(a.first, a.second)))
			continue;
		starts.push_back(a);
	}
	if (!starts.empty()) {
		const auto& s = starts[randRange(rng, static_cast<int>(starts.size()))];
		result.startPositions.push_back({ string::toString(s.first), string::toString(s.second) });
	}

	auto trySurfaceEmitter = [&] (const EntityType& type, int chance) {
		if (chance <= 0)
			return false;
		for (const auto& g : surfaceCells) {
			if (randRange(rng, chance) != 0)
				continue;
			const int px = g.first;
			const int py = g.second - 1;
			if (at(grid, px, py) != Cell::Air)
				continue;
			if (occupied.count(keyXY(px, py)))
				continue;
			if (occupied.count(keyXY(g.first, g.second)))
				continue;
			result.emitters.emplace_back(static_cast<gridCoord>(px), static_cast<gridCoord>(py), type, 1, 0, "");
			occupied.insert(keyXY(px, py));
			return true;
		}
		return false;
	};
	trySurfaceEmitter(EntityTypes::STONE, _rules.stoneChance);
	trySurfaceEmitter(EntityTypes::TREE, _rules.treeChance);
	trySurfaceEmitter(EntityTypes::NPC_WALKING, _rules.walkingChance);
	if (placedTarget) {
		const EntityType& packageType = ThemeTypes::isIce(*_theme) ? EntityTypes::PACKAGE_ICE : EntityTypes::PACKAGE_ROCK;
		trySurfaceEmitter(packageType, _rules.packageChance);
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
	for (int attempt = 0; attempt < 48; ++attempt) {
		std::vector<uint16_t> domain(_width * _height, MASK_ALL);
		std::vector<Cell> grid;
		unsigned int attemptSeed = rng + static_cast<unsigned int>(attempt) * 9973u;
		if (!collapse(domain, grid, attemptSeed))
			continue;
		if (!isAirConnected(grid))
			continue;

		int surfaces = 0;
		for (Cell c : grid)
			if (isWalkableSurface(c))
				++surfaces;
		if (surfaces < 6)
			continue;

		instantiate(grid, result, attemptSeed);
		if (result.startPositions.empty())
			continue;
		if (result.caves.empty() && !_caves.empty())
			continue;
		if (_rules.packageTargetRequired && result.tiles.end() == std::find_if(result.tiles.begin(), result.tiles.end(),
				[] (const MapTileDefinition& t) {
					return t.spriteDef && SpriteTypes::isPackageTarget(t.spriteDef->type);
				}))
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
