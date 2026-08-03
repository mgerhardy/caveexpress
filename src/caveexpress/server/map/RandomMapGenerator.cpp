#include "RandomMapGenerator.h"
#include "common/EmitterDefinition.h"
#include "common/MapSettings.h"
#include "common/Log.h"
#include "common/String.h"
#include "common/LUALibrary.h"
#include "common/ThemeType.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/MapValidator.h"
#include "caveexpress/shared/SpriteShapeTraits.h"
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

uint16_t bit (RandomMapGenerator::Cell c)
{
	return static_cast<uint16_t>(1u << static_cast<unsigned>(c));
}

const uint16_t MASK_AIR = bit(RandomMapGenerator::Cell::Air);
const uint16_t MASK_ROCK = bit(RandomMapGenerator::Cell::Rock);
const uint16_t MASK_FILL = MASK_AIR | MASK_ROCK;
const uint16_t MASK_ALL = static_cast<uint16_t>((1u << static_cast<unsigned>(RandomMapGenerator::Cell::Count)) - 1u);

int keyXY (int x, int y)
{
	return (y << 16) | (x & 0xffff);
}
}

RandomMapRules RandomMapRules::loadFromLua (const std::string& path)
{
	RandomMapRules r;
	LUA lua(false);
	if (!lua.load(path)) {
		Log::info(LOG_GAMEIMPL, "mapgen: no %s, using defaults", path.c_str());
		return r;
	}
	if (!lua.execute("getRandomMapRules", 1)) {
		Log::info(LOG_GAMEIMPL, "mapgen: getRandomMapRules() missing in %s, using defaults", path.c_str());
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
		r.caveNpcDelay = lua.getValueIntegerFromTable("npcDelay", r.caveNpcDelay);
		lua_getfield(L, -1, "npcTypes");
		if (lua_istable(L, -1)) {
			const int n = static_cast<int>(lua_rawlen(L, -1));
			for (int i = 1; i <= n; ++i) {
				lua_rawgeti(L, -1, i);
				if (lua_isstring(L, -1))
					r.caveNpcTypes.push_back(lua_tostring(L, -1));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);
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
		r.minCavePackageAirSeparation = lua.getValueIntegerFromTable("minCaveAirSeparation", r.minCavePackageAirSeparation);
		r.packageTransferCount = lua.getValueIntegerFromTable("transferCount", r.packageTransferCount);
		pop();
	}
	if (getTable("cleanup")) {
		r.minPlatformLength = lua.getValueIntegerFromTable("minPlatformLength", r.minPlatformLength);
		r.minSolidComponentSize = lua.getValueIntegerFromTable("minSolidComponentSize", r.minSolidComponentSize);
		r.minPlatformRows = lua.getValueIntegerFromTable("minPlatformRows", r.minPlatformRows);
		r.minWalkableCells = lua.getValueIntegerFromTable("minWalkableCells", r.minWalkableCells);
		r.minColliderCells = lua.getValueIntegerFromTable("minColliderCells", r.minColliderCells);
		r.minTreeEmitters = lua.getValueIntegerFromTable("minTreeEmitters", r.minTreeEmitters);
		r.minTotalScore = lua.getValueFloatFromTable("minTotalScore", r.minTotalScore);
		r.maxExposedRockTopRatio = lua.getValueFloatFromTable("maxExposedRockTopRatio", r.maxExposedRockTopRatio);
		r.maxOrphanColliderRatio = lua.getValueFloatFromTable("maxOrphanColliderRatio", r.maxOrphanColliderRatio);
		r.maxGenerateAttempts = lua.getValueIntegerFromTable("maxGenerateAttempts", r.maxGenerateAttempts);
		r.minSurfaceCells = lua.getValueIntegerFromTable("minSurfaceCells", r.minSurfaceCells);
		r.minAirPercent = lua.getValueIntegerFromTable("minAirPercent", r.minAirPercent);
		r.requirePlayerStart = lua.getValueBoolFromTable("requirePlayerStart", r.requirePlayerStart);
		r.requireCaveIfAvailable = lua.getValueBoolFromTable("requireCaveIfAvailable", r.requireCaveIfAvailable);
		pop();
	}
	if (getTable("emitters")) {
		r.stoneChance = lua.getValueIntegerFromTable("stoneChance", r.stoneChance);
		r.treeChance = lua.getValueIntegerFromTable("treeChance", r.treeChance);
		r.walkingChance = lua.getValueIntegerFromTable("walkingChance", r.walkingChance);
		r.packageChance = lua.getValueIntegerFromTable("packageChance", r.packageChance);
		r.maxTrees = lua.getValueIntegerFromTable("maxTrees", r.maxTrees);
		pop();
	}
	if (getTable("decor")) {
		r.caveArtChance = lua.getValueIntegerFromTable("caveArtChance", r.caveArtChance);
		pop();
	}
	lua_pop(L, 1);
	return r;
}

bool RandomMapRules::acceptsGrid (int surfaceCells, int airCells, int mapCells) const
{
	if (surfaceCells < minSurfaceCells)
		return false;
	if (mapCells <= 0)
		return false;
	return airCells * 100 >= mapCells * minAirPercent;
}

bool RandomMapRules::accepts (const MapMetrics& metrics, int mapWidth, int mapHeight, float waterHeight) const
{
	const int waterRows = std::max(1, static_cast<int>(std::ceil(waterHeight)));
	const int usableSpan = std::max(1, mapHeight - waterRows - 4);
	const int maxPossiblePlatformRows = std::max(2, usableSpan / std::max(1, minVerticalGap) + 1);
	const int requiredPlatformRows = std::min(minPlatformRows, maxPossiblePlatformRows);
	const int requiredWalkable = std::min(minWalkableCells, mapWidth * mapHeight / 8);
	const int requiredColliders = std::min(minColliderCells, mapWidth * mapHeight / 8);

	return metrics.valid
			&& metrics.unreachableFlyable == 0
			&& metrics.cavesAbovePackageTarget == 0
			&& metrics.cavePackageAirTooClose == 0
			&& metrics.shortPlatformRuns == 0
			&& metrics.smallSolidComponents == 0
			&& metrics.isolatedWalkables == 0
			&& metrics.platformRows >= requiredPlatformRows
			&& metrics.walkableCells >= requiredWalkable
			&& metrics.colliderCells >= requiredColliders
			&& metrics.treeEmitterCount >= minTreeEmitters
			&& metrics.totalScore >= minTotalScore
			&& metrics.exposedRockTopRatio <= maxExposedRockTopRatio
			&& metrics.orphanColliderRatio <= maxOrphanColliderRatio
			&& metrics.windowWindowAdjacencies == 0
			&& metrics.cavesTooClose == 0
			&& metrics.packageTargetsWithBadNiche == 0
			&& metrics.bridgesWithoutBackground == 0;
}

RandomMapGenerator::RandomMapGenerator (const ThemeType& theme, unsigned int width, unsigned int height,
		const RandomMapRules& rules, float waterHeight) :
		_theme(&theme), _width(std::max(10u, width)), _height(std::max(8u, height)), _rules(rules)
{
	_rules.caveTarget = std::max(1, _rules.caveTarget);
	if (waterHeight > 0.0f)
		_waterHeight = waterHeight;
	else
		_waterHeight = std::min(2.5f, std::max(1.0f, static_cast<float>(_height) * 0.18f));
	loadEntitySizesFromLua();
	collectSprites();
	resolveGameplayKit();
}

const EntityType* RandomMapGenerator::resolvePackageEntity (const ThemeType& theme)
{
	const std::string themedName = "item-package-" + theme.name;
	const EntityType& themed = EntityType::getByName(themedName);
	if (!themed.isNone())
		return &themed;
	if (ThemeTypes::isRock(theme))
		return &EntityTypes::PACKAGE_ROCK;
	return nullptr;
}

void RandomMapGenerator::resolveGameplayKit ()
{
	_kit.caveNpcDelay = std::max(0, _rules.caveNpcDelay);
	_kit.caveNpcs.clear();
	if (_rules.caveNpcTypes.empty()) {
		_kit.caveNpcs.push_back(&EntityTypes::NPC_FRIENDLY_MAN);
		_kit.caveNpcs.push_back(&EntityTypes::NPC_FRIENDLY_WOMAN);
		_kit.caveNpcs.push_back(&EntityTypes::NPC_FRIENDLY_GRANDPA);
	} else {
		for (const std::string& name : _rules.caveNpcTypes) {
			const EntityType& t = EntityType::getByName(name);
			if (!t.isNone())
				_kit.caveNpcs.push_back(&t);
		}
		if (_kit.caveNpcs.empty()) {
			_kit.caveNpcs.push_back(&EntityTypes::NPC_FRIENDLY_MAN);
			_kit.caveNpcs.push_back(&EntityTypes::NPC_FRIENDLY_WOMAN);
			_kit.caveNpcs.push_back(&EntityTypes::NPC_FRIENDLY_GRANDPA);
		}
	}

	const bool hasTargets = !_packageTargets.empty();
	if (hasTargets && _rules.packageTargetRequired) {
		_kit.packageTargetRequired = true;
		_kit.packageEntity = resolvePackageEntity(*_theme);
		_kit.packageTransferCount = std::max(1, _rules.packageTransferCount);
		_kit.npcTransferCount = 0;
	} else {
		_kit.packageTargetRequired = false;
		_kit.packageEntity = hasTargets ? resolvePackageEntity(*_theme) : nullptr;
		_kit.packageTransferCount = 0;
		_kit.npcTransferCount = std::max(1, _rules.caveTarget);
		if (_rules.packageTargetRequired && !hasTargets) {
			Log::info(LOG_GAMEIMPL,
					"mapgen: theme '%s' has no package-target sprites — NPC-transfer mode",
					_theme->name.c_str());
		}
	}
}

bool RandomMapGenerator::isWalkableSurface (Cell c) const
{
	return c == Cell::Ground || c == Cell::LedgeL || c == Cell::LedgeR || c == Cell::Bridge
			|| c == Cell::SlopeL || c == Cell::SlopeR;
}

bool RandomMapGenerator::isColliderSolid (Cell c) const
{
	return c == Cell::Rock || c == Cell::UndercutL || c == Cell::UndercutR || c == Cell::Shim
			|| isWalkableSurface(c);
}

bool RandomMapGenerator::isShimSprite (const SpriteDefPtr& def) const
{
	return analyzeSpriteShape(def).shim;
}

bool RandomMapGenerator::isHangingGroundSprite (const SpriteDefPtr& def) const
{
	return analyzeSpriteShape(def).thinTopSlab;
}

bool RandomMapGenerator::isUndercutLeftSprite (const SpriteDefPtr& def) const
{
	return analyzeSpriteShape(def).undercutL;
}

bool RandomMapGenerator::isUndercutRightSprite (const SpriteDefPtr& def) const
{
	return analyzeSpriteShape(def).undercutR;
}

bool RandomMapGenerator::isFullRockSprite (const SpriteDefPtr& def) const
{
	if (!SpriteTypes::isRock(def->type))
		return false;
	if (def->width > 1.01f || def->height > 1.01f)
		return false;
	const SpriteShapeTraits traits = analyzeSpriteShape(def);
	if (traits.shim || traits.undercutL || traits.undercutR || traits.slopeL || traits.slopeR)
		return false;
	return traits.fullSolid;
}

void RandomMapGenerator::collectSprites ()
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

		const SpriteShapeTraits traits = analyzeSpriteShape(def);
		if (traits.slopeL)
			_slopesL.push_back(def);
		else if (traits.slopeR)
			_slopesR.push_back(def);
		else if (traits.shim)
			_shims.push_back(def);
		else if (traits.undercutL)
			_undercutL.push_back(def);
		else if (traits.undercutR)
			_undercutR.push_back(def);
		else if (traits.fullSolid && SpriteTypes::isRock(type))
			_rocksFull.push_back(def);
		else if (SpriteTypes::isGround(type)) {
			if (traits.thinTopSlab)
				_groundsHanging.push_back(def);
			else
				_grounds.push_back(def);
		} else if (SpriteTypes::isGroundLeft(type))
			_groundLeft.push_back(def);
		else if (SpriteTypes::isGroundRight(type))
			_groundRight.push_back(def);
		else if (SpriteTypes::isBackground(type)) {
			_backgrounds.push_back(def);
			if (def->id.find("cave-art") != std::string::npos)
				_caveArt.push_back(def);
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
	if (_backgrounds.empty())
		_backgrounds = _grounds;
	if (_bridgePlank.empty())
		_bridgePlank = _bridgeLeft;
	Log::info(LOG_GAMEIMPL, "mapgen sprites rock=%i ground=%i hang=%i slope=%i/%i undercut=%i/%i shim=%i bridge=%i/%i/%i pkgTarget=%i cave=%i caveArt=%i",
			(int)_rocksFull.size(), (int)_grounds.size(), (int)_groundsHanging.size(),
			(int)_slopesL.size(), (int)_slopesR.size(),
			(int)_undercutL.size(), (int)_undercutR.size(), (int)_shims.size(),
			(int)_bridgeLeft.size(), (int)_bridgePlank.size(),
			(int)_bridgeRight.size(), (int)_packageTargets.size(), (int)_caves.size(),
			(int)_caveArt.size());
}

RandomMapGenerator::SpriteBucketCounts RandomMapGenerator::spriteBucketCounts () const
{
	SpriteBucketCounts c;
	c.rocksFull = static_cast<int>(_rocksFull.size());
	c.grounds = static_cast<int>(_grounds.size());
	c.groundsHanging = static_cast<int>(_groundsHanging.size());
	c.undercutL = static_cast<int>(_undercutL.size());
	c.undercutR = static_cast<int>(_undercutR.size());
	c.shims = static_cast<int>(_shims.size());
	c.slopesL = static_cast<int>(_slopesL.size());
	c.slopesR = static_cast<int>(_slopesR.size());
	c.bridges = static_cast<int>(_bridgeLeft.size() + _bridgeRight.size() + _bridgePlank.size());
	c.caves = static_cast<int>(_caves.size());
	c.backgrounds = static_cast<int>(_backgrounds.size());
	c.caveArt = static_cast<int>(_caveArt.size());
	c.packageTargets = static_cast<int>(_packageTargets.size());
	return c;
}

bool RandomMapGenerator::compatible (Cell a, Cell b, int dir) const
{
	auto walkable = [this] (Cell c) { return isWalkableSurface(c); };
	auto solid = [this] (Cell c) { return isColliderSolid(c); };
	auto support = [this] (Cell c) {
		return c == Cell::Rock || isWalkableSurface(c);
	};

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
				|| b == Cell::SlopeL || b == Cell::SlopeR
				|| b == Cell::Rock || b == Cell::Air;

	case Cell::Bridge:
		// Bridges sit over open air (background on the same cell) and only connect to ground/bridge.
		if (dir == 0 || dir == 2)
			return b == Cell::Air;
		return b == Cell::Ground || b == Cell::Bridge || b == Cell::LedgeL || b == Cell::LedgeR
				|| b == Cell::SlopeL || b == Cell::SlopeR;

	case Cell::LedgeL:
		if (dir == 3)
			return b == Cell::Air;
		if (dir == 1)
			return b == Cell::Ground || b == Cell::Bridge || b == Cell::LedgeR || b == Cell::LedgeL
					|| b == Cell::SlopeL || b == Cell::SlopeR;
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return b == Cell::Air; // overhang - never solid fill under ledge sprites
		break;

	case Cell::LedgeR:
		if (dir == 1)
			return b == Cell::Air;
		if (dir == 3)
			return b == Cell::Ground || b == Cell::Bridge || b == Cell::LedgeL || b == Cell::LedgeR
					|| b == Cell::SlopeL || b == Cell::SlopeR;
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return b == Cell::Air;
		break;

	case Cell::SlopeL:
		// Bottom-right fill: thick on right, open upper-left.
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return solid(b);
		if (dir == 1)
			return support(b);
		if (dir == 3)
			return b == Cell::Air || walkable(b) || b == Cell::Rock;
		break;

	case Cell::SlopeR:
		// Bottom-left fill: thick on left, open upper-right.
		if (dir == 0)
			return b == Cell::Air;
		if (dir == 2)
			return solid(b);
		if (dir == 3)
			return support(b);
		if (dir == 1)
			return b == Cell::Air || walkable(b) || b == Cell::Rock;
		break;

	case Cell::UndercutL:
		// Top/right-biased bite: must hang under support, open lower-left.
		if (dir == 0)
			return support(b);
		if (dir == 2)
			return b == Cell::Air;
		if (dir == 1)
			return b == Cell::Rock || b == Cell::UndercutL || b == Cell::UndercutR;
		if (dir == 3)
			return b == Cell::Air;
		break;

	case Cell::UndercutR:
		if (dir == 0)
			return support(b);
		if (dir == 2)
			return b == Cell::Air;
		if (dir == 3)
			return b == Cell::Rock || b == Cell::UndercutL || b == Cell::UndercutR;
		if (dir == 1)
			return b == Cell::Air;
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

int RandomMapGenerator::entropy (uint16_t mask) const
{
	int n = 0;
	for (unsigned i = 0; i < static_cast<unsigned>(Cell::Count); ++i)
		if (mask & (1u << i))
			++n;
	return n;
}

RandomMapGenerator::Cell RandomMapGenerator::observe (uint16_t mask, unsigned int& rng) const
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
	if (!_undercutL.empty())
		add(Cell::UndercutL, _rules.weightUndercut);
	if (!_undercutR.empty())
		add(Cell::UndercutR, _rules.weightUndercut);
	if (!_shims.empty())
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

void RandomMapGenerator::applyBorderSeeds (std::vector<uint16_t>& domain) const
{
	const int bedStart = waterBedStartRow();
	for (unsigned y = 0; y < _height; ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			uint16_t& cell = domain[x + y * _width];
			if (y < 2)
				cell = MASK_AIR;
			else if (static_cast<int>(y) >= bedStart)
				cell = MASK_ROCK;
			else if (x == 0 || x == _width - 1)
				cell = MASK_AIR | MASK_ROCK;
			else
				cell = MASK_FILL;
		}
	}
}

float RandomMapGenerator::waterSurfaceY () const
{
	return static_cast<float>(_height) - _waterHeight;
}

int RandomMapGenerator::waterBedStartRow () const
{
	return static_cast<int>(_height) - std::max(1, static_cast<int>(std::ceil(_waterHeight)));
}

int RandomMapGenerator::maxWalkableRow () const
{
	// Walkable tile top is at row y; water sensor starts at waterSurfaceY().
	// Keep at least one full cell of clearance so physics never sees ground on the waterline.
	return static_cast<int>(std::floor(waterSurfaceY() - 1.0f + 1e-4f));
}

void RandomMapGenerator::enforceWaterClearance (std::vector<Cell>& grid) const
{
	const int maxWalk = maxWalkableRow();
	const int bedStart = waterBedStartRow();
	for (unsigned y = 0; y < _height; ++y) {
		if (static_cast<int>(y) <= maxWalk)
			continue;
		for (unsigned x = 0; x < _width; ++x) {
			Cell& c = grid[x + y * _width];
			if (!isWalkableSurface(c))
				continue;
			c = (static_cast<int>(y) >= bedStart) ? Cell::Rock : Cell::Air;
		}
	}
}

void RandomMapGenerator::seedPlatforms (std::vector<uint16_t>& domain, unsigned int& rng) const
{
	const int minY = 2;
	const int maxY = std::min(waterBedStartRow() - 2, maxWalkableRow());
	if (maxY <= minY)
		return;

	// Never let platform rock fill fuse into the water bed or waterline clearance.
	const int fillLimitY = maxWalkableRow() + 1;

	const int bandSpan = std::max(0, _rules.platformBandMax - _rules.platformBandMin);
	const int bandCount = _rules.platformBandMin + randRange(rng, bandSpan + 1);
	std::vector<int> rows;
	int guard = 0;
	while (static_cast<int>(rows.size()) < bandCount && guard++ < 80) {
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
		int x = 1 + randRange(rng, 3);
		while (x < static_cast<int>(_width) - 3) {
			const int len = _rules.lengthMin + randRange(rng, lenSpan + 1);
			if (x + len >= static_cast<int>(_width) - 1)
				break;
			const bool floating = randRange(rng, 1000) < static_cast<int>(_rules.floatingChance * 1000.0f);
			int fillDepth = floating ? 0 : (1 + randRange(rng, 3));
			// Keep an air gap above the water bed so walls stay distinct platforms.
			while (fillDepth > 0 && y + fillDepth >= fillLimitY)
				--fillDepth;

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
					if (cy < fillLimitY)
						domain[cx + cy * _width] = MASK_AIR;
					continue;
				}

				for (int d = 1; d <= fillDepth; ++d) {
					const int cy = y + d;
					if (cy >= fillLimitY)
						break;
					domain[cx + cy * _width] = MASK_ROCK;
				}
			}
			x += len + _rules.gapMin + randRange(rng, gapSpan + 1);
		}
	}
}

bool RandomMapGenerator::propagate (std::vector<uint16_t>& domain, int startX, int startY) const
{
	std::queue<int> q;
	std::vector<uint8_t> inQueue(domain.size(), 0);
	const int startIdx = startX + startY * static_cast<int>(_width);
	q.push(startIdx);
	inQueue[startIdx] = 1;

	while (!q.empty()) {
		const int idx = q.front();
		q.pop();
		inQueue[idx] = 0;
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
			const int opposite = (d + 2) % 4;
			for (unsigned a = 0; a < static_cast<unsigned>(Cell::Count); ++a) {
				if ((mask & (1u << a)) == 0)
					continue;
				for (unsigned b = 0; b < static_cast<unsigned>(Cell::Count); ++b) {
					if ((nmask & (1u << b)) == 0)
						continue;
					const Cell ca = static_cast<Cell>(a);
					const Cell cb = static_cast<Cell>(b);
					// Both directions must agree.
					if (compatible(ca, cb, d) && compatible(cb, ca, opposite))
						allowed = static_cast<uint16_t>(allowed | (1u << b));
				}
			}
			const uint16_t narrowed = static_cast<uint16_t>(nmask & allowed);
			if (narrowed == nmask)
				continue;
			if (narrowed == 0)
				return false;
			nmask = narrowed;
			if (!inQueue[nidx]) {
				q.push(nidx);
				inQueue[nidx] = 1;
			}
		}
	}
	return true;
}

void RandomMapGenerator::decorateEdges (std::vector<Cell>& grid, unsigned int& rng) const
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
				// Undercut only when the open side is air and the thick side has rock support.
				if (!_undercutL.empty() && left == Cell::Air && right == Cell::Rock)
					next[x + y * _width] = Cell::UndercutL;
				else if (!_undercutR.empty() && right == Cell::Air && left == Cell::Rock)
					next[x + y * _width] = Cell::UndercutR;
			}

			if (!_shims.empty() && above == Cell::Rock && below == Cell::Air
					&& (left == Cell::Air || right == Cell::Air) && randRange(rng, 3) == 0)
				next[x + y * _width] = Cell::Shim;
		}
	}
	grid.swap(next);
}

void RandomMapGenerator::decorateSlopes (std::vector<Cell>& grid) const
{
	if (_slopesL.empty() && _slopesR.empty())
		return;

	std::vector<Cell> next = grid;
	for (unsigned y = 1; y + 1 < _height; ++y) {
		for (unsigned x = 1; x + 1 < _width; ++x) {
			const Cell c = grid[x + y * _width];
			if (c != Cell::Ground && c != Cell::LedgeL && c != Cell::LedgeR)
				continue;

			const Cell above = at(grid, static_cast<int>(x), static_cast<int>(y) - 1, Cell::Air);
			const Cell below = at(grid, static_cast<int>(x), static_cast<int>(y) + 1, Cell::Air);
			const Cell left = at(grid, static_cast<int>(x) - 1, static_cast<int>(y), Cell::Air);
			const Cell right = at(grid, static_cast<int>(x) + 1, static_cast<int>(y), Cell::Air);
			if (above != Cell::Air)
				continue;

			const bool leftSupport = left == Cell::Rock || isWalkableSurface(left);
			const bool rightSupport = right == Cell::Rock || isWalkableSurface(right);
			const bool belowSolid = isColliderSolid(below);
			const bool belowLeftSolid = isColliderSolid(at(grid, static_cast<int>(x) - 1, static_cast<int>(y) + 1, Cell::Air));
			const bool belowRightSolid = isColliderSolid(at(grid, static_cast<int>(x) + 1, static_cast<int>(y) + 1, Cell::Air));

			// Right cliff edge → slope-right (thick on left, open upper-right).
			if (!_slopesR.empty() && leftSupport && right == Cell::Air && (belowSolid || belowLeftSolid))
				next[x + y * _width] = Cell::SlopeR;
			// Left cliff edge → slope-left (thick on right, open upper-left).
			else if (!_slopesL.empty() && rightSupport && left == Cell::Air && (belowSolid || belowRightSolid))
				next[x + y * _width] = Cell::SlopeL;
		}
	}
	grid.swap(next);
}

void RandomMapGenerator::ensureWalkableTops (std::vector<Cell>& grid) const
{
	const int minLen = std::max(1, _rules.minPlatformLength);
	const int maxWalk = maxWalkableRow();
	for (unsigned y = 1; y < _height; ++y) {
		if (static_cast<int>(y) > maxWalk)
			continue;
		unsigned x = 0;
		while (x < _width) {
			const Cell c = grid[x + y * _width];
			const Cell above = at(grid, static_cast<int>(x), static_cast<int>(y) - 1, Cell::Air);
			const bool candidate = above == Cell::Air
					&& (c == Cell::Rock || c == Cell::UndercutL || c == Cell::UndercutR || c == Cell::Shim
							|| c == Cell::Ground || c == Cell::LedgeL || c == Cell::LedgeR
							|| c == Cell::SlopeL || c == Cell::SlopeR);
			if (!candidate) {
				++x;
				continue;
			}
			const unsigned start = x;
			while (x < _width) {
				const Cell cc = grid[x + y * _width];
				const Cell aa = at(grid, static_cast<int>(x), static_cast<int>(y) - 1, Cell::Air);
				const bool ok = aa == Cell::Air
						&& (cc == Cell::Rock || cc == Cell::UndercutL || cc == Cell::UndercutR || cc == Cell::Shim
								|| cc == Cell::Ground || cc == Cell::LedgeL || cc == Cell::LedgeR
								|| cc == Cell::SlopeL || cc == Cell::SlopeR);
				if (!ok)
					break;
				++x;
			}
			const unsigned len = x - start;
			if (static_cast<int>(len) >= minLen) {
				for (unsigned i = start; i < x; ++i) {
					const Cell prev = grid[i + y * _width];
					// Preserve dedicated slope cells already placed at edges.
					if (prev != Cell::SlopeL && prev != Cell::SlopeR)
						grid[i + y * _width] = Cell::Ground;
				}
			} else {
				// Too short: shave the protrusion away instead of leaving a 1-2 tile stub.
				for (unsigned i = start; i < x; ++i)
					grid[i + y * _width] = Cell::Air;
			}
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

void RandomMapGenerator::removeShortPlatforms (std::vector<Cell>& grid) const
{
	const int minLen = std::max(1, _rules.minPlatformLength);
	const int waterRows = std::max(1, static_cast<int>(std::ceil(_waterHeight)));
	const int bottomY = static_cast<int>(_height) - waterRows;
	for (unsigned y = 0; y < _height; ++y) {
		unsigned x = 0;
		while (x < _width) {
			if (!isWalkableSurface(grid[x + y * _width]) || grid[x + y * _width] == Cell::Bridge) {
				++x;
				continue;
			}
			const unsigned start = x;
			while (x < _width && isWalkableSurface(grid[x + y * _width]) && grid[x + y * _width] != Cell::Bridge)
				++x;
			const unsigned len = x - start;
			if (static_cast<int>(len) >= minLen)
				continue;
			for (unsigned i = start; i < x; ++i) {
				grid[i + y * _width] = Cell::Air;
				// Shave 1-wide rock pillars that would just be re-capped.
				int cy = static_cast<int>(y) + 1;
				while (cy < bottomY) {
					const Cell s = at(grid, static_cast<int>(i), cy, Cell::Air);
					if (!isColliderSolid(s) || s == Cell::Bridge)
						break;
					const Cell left = at(grid, static_cast<int>(i) - 1, cy, Cell::Air);
					const Cell right = at(grid, static_cast<int>(i) + 1, cy, Cell::Air);
					if (isColliderSolid(left) || isColliderSolid(right))
						break;
					grid[i + cy * _width] = Cell::Air;
					++cy;
				}
			}
		}
	}
}

void RandomMapGenerator::placeBridgeSpans (std::vector<Cell>& grid, unsigned int& rng) const
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
				const bool leftOk = isWalkableSurface(leftAnchor) && leftAnchor != Cell::Bridge;
				const bool rightOk = isWalkableSurface(rightAnchor) && rightAnchor != Cell::Bridge;
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

void RandomMapGenerator::removeSmallSolidComponents (std::vector<Cell>& grid) const
{
	const int n = static_cast<int>(_width * _height);
	const int waterRows = std::max(1, static_cast<int>(std::ceil(_waterHeight)));
	const int bottomY = static_cast<int>(_height) - waterRows;
	std::vector<int> comp(n, -1);
	std::vector<int> sizes;
	std::vector<uint8_t> touchesBottom;
	std::vector<int> stack;
	int nextId = 0;

	for (int y = 0; y < static_cast<int>(_height); ++y) {
		for (int x = 0; x < static_cast<int>(_width); ++x) {
			const int start = x + y * static_cast<int>(_width);
			if (comp[start] >= 0 || !isColliderSolid(grid[start]))
				continue;
			stack.clear();
			stack.push_back(start);
			comp[start] = nextId;
			int size = 0;
			bool bottom = false;
			while (!stack.empty()) {
				const int i = stack.back();
				stack.pop_back();
				++size;
				const int cx = i % static_cast<int>(_width);
				const int cy = i / static_cast<int>(_width);
				if (cy >= bottomY)
					bottom = true;
				for (int d = 0; d < 4; ++d) {
					const int nx = cx + DIR[d][0];
					const int ny = cy + DIR[d][1];
					if (nx < 0 || ny < 0 || nx >= static_cast<int>(_width) || ny >= static_cast<int>(_height))
						continue;
					const int ni = nx + ny * static_cast<int>(_width);
					if (comp[ni] >= 0 || !isColliderSolid(grid[ni]))
						continue;
					comp[ni] = nextId;
					stack.push_back(ni);
				}
			}
			sizes.push_back(size);
			touchesBottom.push_back(bottom ? 1 : 0);
			++nextId;
		}
	}

	for (int i = 0; i < n; ++i) {
		const int id = comp[i];
		if (id < 0)
			continue;
		if (touchesBottom[id])
			continue;
		if (sizes[id] >= _rules.minSolidComponentSize)
			continue;
		grid[i] = Cell::Air;
	}
}

void RandomMapGenerator::fillEnclosedAirPockets (std::vector<Cell>& grid) const
{
	// Single air cells fully surrounded by solids look like swiss-cheese clutter.
	std::vector<Cell> next = grid;
	for (unsigned y = 1; y + 1 < _height; ++y) {
		for (unsigned x = 1; x + 1 < _width; ++x) {
			if (grid[x + y * _width] != Cell::Air)
				continue;
			bool enclosed = true;
			for (int d = 0; d < 4; ++d) {
				const Cell n = at(grid, static_cast<int>(x) + DIR[d][0], static_cast<int>(y) + DIR[d][1], Cell::Air);
				if (!isColliderSolid(n)) {
					enclosed = false;
					break;
				}
			}
			if (enclosed)
				next[x + y * _width] = Cell::Rock;
		}
	}
	grid.swap(next);
}

void RandomMapGenerator::relabelPlatformEdges (std::vector<Cell>& grid) const
{
	for (unsigned y = 0; y < _height; ++y) {
		unsigned x = 0;
		while (x < _width) {
			Cell c = grid[x + y * _width];
			if (!(c == Cell::Ground || c == Cell::LedgeL || c == Cell::LedgeR)) {
				++x;
				continue;
			}
			const unsigned start = x;
			while (x < _width) {
				c = grid[x + y * _width];
				if (!(c == Cell::Ground || c == Cell::LedgeL || c == Cell::LedgeR))
					break;
				++x;
			}
			const unsigned end = x;
			const unsigned len = end - start;
			if (len == 0)
				continue;
			for (unsigned i = start; i < end; ++i) {
				if (i == start && len >= 2)
					grid[i + y * _width] = Cell::LedgeL;
				else if (i + 1 == end && len >= 2)
					grid[i + y * _width] = Cell::LedgeR;
				else
					grid[i + y * _width] = Cell::Ground;
			}
			// Ledges overhang: force air beneath ends.
			if (len >= 2 && y + 1 < _height) {
				grid[start + (y + 1) * _width] = Cell::Air;
				grid[(end - 1) + (y + 1) * _width] = Cell::Air;
			}
		}
	}
}

void RandomMapGenerator::cleanupClutter (std::vector<Cell>& grid) const
{
	// Iterate a few times: removing blobs can create short platforms and vice versa.
	for (int pass = 0; pass < 3; ++pass) {
		removeSmallSolidComponents(grid);
		removeShortPlatforms(grid);
		fillEnclosedAirPockets(grid);
		ensureWalkableTops(grid);
	}
	relabelPlatformEdges(grid);
	enforceWaterClearance(grid);
}

bool RandomMapGenerator::collapse (std::vector<uint16_t>& domain, std::vector<Cell>& out, unsigned int& rng) const
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
			Cell::LedgeL, Cell::LedgeR, Cell::SlopeL, Cell::SlopeR, Cell::Ground, Cell::Bridge,
			Cell::UndercutL, Cell::UndercutR, Cell::Shim, Cell::Rock, Cell::Air
		};
		for (Cell c : preference) {
			if ((domain[i] & bit(c)) == 0)
				continue;
			if (c == Cell::UndercutL && _undercutL.empty())
				continue;
			if (c == Cell::UndercutR && _undercutR.empty())
				continue;
			if (c == Cell::Shim && _shims.empty())
				continue;
			resolved = c;
			break;
		}
		out[i] = resolved;
	}

	decorateEdges(out, rng);
	ensureWalkableTops(out);
	cleanupClutter(out);
	placeBridgeSpans(out, rng);
	ensureWalkableTops(out);
	relabelPlatformEdges(out);
	fillUnreachableAir(out);
	removeSmallSolidComponents(out);
	removeShortPlatforms(out);
	ensureWalkableTops(out);
	relabelPlatformEdges(out);
	decorateSlopes(out);
	fillUnreachableAir(out);
	removeSmallSolidComponents(out);
	enforceWaterClearance(out);
	return true;
}

int RandomMapGenerator::findOpenAirSeed (const std::vector<Cell>& grid) const
{
	// Prefer top-band air (open sky), then any air cell.
	for (unsigned y = 0; y < std::min(2u, _height); ++y) {
		for (unsigned x = 0; x < _width; ++x) {
			if (grid[x + y * _width] == Cell::Air)
				return static_cast<int>(x + y * _width);
		}
	}
	for (unsigned i = 0; i < grid.size(); ++i) {
		if (grid[i] == Cell::Air)
			return static_cast<int>(i);
	}
	return -1;
}

void RandomMapGenerator::floodAir (const std::vector<Cell>& grid, int startIdx, std::vector<uint8_t>& seen, int& reached) const
{
	seen.assign(grid.size(), 0);
	reached = 0;
	if (startIdx < 0 || startIdx >= static_cast<int>(grid.size()) || grid[startIdx] != Cell::Air)
		return;

	std::queue<int> q;
	q.push(startIdx);
	seen[startIdx] = 1;
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
}

void RandomMapGenerator::fillUnreachableAir (std::vector<Cell>& grid) const
{
	const int seed = findOpenAirSeed(grid);
	if (seed < 0)
		return;

	std::vector<uint8_t> seen;
	int reached = 0;
	floodAir(grid, seed, seen, reached);

	bool filled = false;
	for (unsigned i = 0; i < grid.size(); ++i) {
		if (grid[i] != Cell::Air || seen[i])
			continue;
		grid[i] = Cell::Rock;
		filled = true;
	}
	if (filled)
		ensureWalkableTops(grid);
}

bool RandomMapGenerator::isAirConnected (const std::vector<Cell>& grid) const
{
	int airCount = 0;
	for (Cell c : grid) {
		if (c == Cell::Air)
			++airCount;
	}
	if (airCount == 0)
		return false;

	const int seed = findOpenAirSeed(grid);
	if (seed < 0)
		return false;

	std::vector<uint8_t> seen;
	int reached = 0;
	floodAir(grid, seed, seen, reached);
	return reached == airCount;
}

int RandomMapGenerator::airPathDistance (const std::vector<Cell>& grid, int sx, int sy, int gx, int gy) const
{
	if (sx < 0 || sy < 0 || gx < 0 || gy < 0)
		return -1;
	if (sx >= static_cast<int>(_width) || sy >= static_cast<int>(_height)
			|| gx >= static_cast<int>(_width) || gy >= static_cast<int>(_height))
		return -1;
	if (grid[sx + sy * _width] != Cell::Air || grid[gx + gy * _width] != Cell::Air)
		return -1;
	if (sx == gx && sy == gy)
		return 0;

	std::vector<int> dist(grid.size(), -1);
	std::queue<int> q;
	const int start = sx + sy * static_cast<int>(_width);
	const int goal = gx + gy * static_cast<int>(_width);
	dist[start] = 0;
	q.push(start);
	while (!q.empty()) {
		const int idx = q.front();
		q.pop();
		if (idx == goal)
			return dist[idx];
		const int cx = idx % static_cast<int>(_width);
		const int cy = idx / static_cast<int>(_width);
		for (int d = 0; d < 4; ++d) {
			const int nx = cx + DIR[d][0];
			const int ny = cy + DIR[d][1];
			if (nx < 0 || ny < 0 || nx >= static_cast<int>(_width) || ny >= static_cast<int>(_height))
				continue;
			const int nidx = nx + ny * static_cast<int>(_width);
			if (dist[nidx] >= 0 || grid[nidx] != Cell::Air)
				continue;
			dist[nidx] = dist[idx] + 1;
			q.push(nidx);
		}
	}
	return -1;
}

RandomMapGenerator::Cell RandomMapGenerator::at (const std::vector<Cell>& grid, int x, int y, Cell fallback) const
{
	if (x < 0 || y < 0 || x >= static_cast<int>(_width) || y >= static_cast<int>(_height))
		return fallback;
	return grid[x + y * _width];
}

SpriteDefPtr RandomMapGenerator::pick (const std::vector<SpriteDefPtr>& list, unsigned int& rng) const
{
	if (list.empty())
		return SpriteDefPtr();
	return list[randRange(rng, static_cast<int>(list.size()))];
}

void RandomMapGenerator::setTile (Result& result, int x, int y, const SpriteDefPtr& def) const
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

void RandomMapGenerator::instantiate (const std::vector<Cell>& grid, Result& result, unsigned int& rng) const
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
				// Thin top-slab grounds only when air is beneath and a solid neighbor anchors the slab.
				const Cell below = at(grid, static_cast<int>(x), static_cast<int>(y) + 1);
				const Cell left = at(grid, static_cast<int>(x) - 1, static_cast<int>(y));
				const Cell right = at(grid, static_cast<int>(x) + 1, static_cast<int>(y));
				const bool anchored = isColliderSolid(left) || isColliderSolid(right);
				if (below == Cell::Air && anchored && !_groundsHanging.empty() && randRange(rng, 4) == 0)
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
			case Cell::SlopeL:
				def = pick(_slopesL.empty() ? _grounds : _slopesL, rng);
				surfaceCells.emplace_back(static_cast<int>(x), static_cast<int>(y));
				break;
			case Cell::SlopeR:
				def = pick(_slopesR.empty() ? _grounds : _slopesR, rng);
				surfaceCells.emplace_back(static_cast<int>(x), static_cast<int>(y));
				break;
			case Cell::Bridge: {
				const Cell left = at(grid, static_cast<int>(x) - 1, static_cast<int>(y));
				const Cell right = at(grid, static_cast<int>(x) + 1, static_cast<int>(y));
				const bool leftWalk = isWalkableSurface(left);
				const bool rightWalk = isWalkableSurface(right);
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

	// Package target first so caves can avoid stacking above it / sitting too close by air path.
	bool placedTarget = false;
	std::pair<int, int> packageTargetPos(-1, -1);
	auto isFlatEmitterSupport = [] (Cell c) {
		// Slopes are walkable but not stable footing for NPCs/trees/package targets.
		return c == Cell::Ground || c == Cell::LedgeL || c == Cell::LedgeR;
	};
	auto tryPlacePackageTarget = [&] (bool requireWalkableSides) {
		for (const auto& g : surfaceCells) {
			const int x = g.first;
			const int y = g.second;
			if (!isFlatEmitterSupport(at(grid, x, y)))
				continue;
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
			if (requireWalkableSides) {
				if (!isFlatEmitterSupport(left) || !isFlatEmitterSupport(right))
					continue;
			} else if (!isColliderSolid(left) || !isColliderSolid(right)) {
				continue;
			}
			setTile(result, x, y, pick(_packageTargets, rng));
			occupied.insert(keyXY(x, y));
			packageTargetPos = { x, y };
			placedTarget = true;
			return true;
		}
		return false;
	};
	if (!_packageTargets.empty()) {
		tryPlacePackageTarget(_rules.packageTargetSidesWalkable);
		if (_kit.packageTargetRequired && !placedTarget)
			tryPlacePackageTarget(false);
	}

	unsigned cavesPlaced = 0;
	std::vector<std::pair<int, int>> placedCaves;
	for (const auto& g : surfaceCells) {
		if (cavesPlaced >= static_cast<unsigned int>(_rules.caveTarget))
			break;
		const int cx = g.first;
		const int cy = g.second - 1;
		// Cave NPCs walk out onto the tile below — slopes dump them into the void.
		if (!isFlatEmitterSupport(at(grid, g.first, g.second)))
			continue;
		if (cy < 1 || at(grid, cx, cy) != Cell::Air)
			continue;
		if (occupied.count(keyXY(cx, cy)))
			continue;
		// Never put a cave entrance directly above a package target (same column).
		if (placedTarget && cx == packageTargetPos.first && cy < packageTargetPos.second)
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

		// Require a minimum flyable air-path to the package approach cell. A wall or
		// ground gap that forces a detour increases this distance (desired gameplay).
		if (placedTarget && _rules.minCavePackageAirSeparation > 0) {
			const int tx = packageTargetPos.first;
			const int ty = packageTargetPos.second - 1;
			if (ty >= 0 && at(grid, tx, ty) == Cell::Air) {
				const int airDist = airPathDistance(grid, cx, cy, tx, ty);
				if (airDist >= 0 && airDist < _rules.minCavePackageAirSeparation)
					continue;
			}
		}

		const SpriteDefPtr cave = pick(_caves, rng);
		if (!cave)
			break;
		result.tiles.erase(std::remove_if(result.tiles.begin(), result.tiles.end(),
				[cx, cy] (const MapTileDefinition& t) {
					return static_cast<int>(t.x) == cx && static_cast<int>(t.y) == cy;
				}), result.tiles.end());

		const EntityType* npc = &EntityType::NONE;
		if (!_kit.caveNpcs.empty()
				&& randRange(rng, 1000) < static_cast<int>(_rules.caveNpcChance * 1000.0f)) {
			npc = _kit.caveNpcs[randRange(rng, static_cast<int>(_kit.caveNpcs.size()))];
		}
		result.caves.emplace_back(static_cast<gridCoord>(cx), static_cast<gridCoord>(cy), cave, *npc,
				_kit.caveNpcDelay);
		occupied.insert(keyXY(cx, cy));
		placedCaves.emplace_back(cx, cy);
		++cavesPlaced;

		if (_rules.windowsEnabled && !_windows.empty()) {
			int firstSide = randRange(rng, 2) == 0 ? -1 : 1;
			const int sideOrder[2] = { firstSide, -firstSide };
			const int sideTries = _rules.oneWindowPerCave ? 1 : 2;
			for (int si = 0; si < sideTries; ++si) {
				const int wx = cx + sideOrder[si];
				if (wx <= 0 || wx >= static_cast<int>(_width) - 1 || at(grid, wx, cy) != Cell::Air)
					continue;
				if (!isFlatEmitterSupport(at(grid, wx, cy + 1)) || occupied.count(keyXY(wx, cy)))
					continue;
				auto hasWindowAt = [&] (int x, int y) {
					for (const MapTileDefinition& t : result.tiles) {
						if (static_cast<int>(t.x) == x && static_cast<int>(t.y) == y && t.spriteDef
								&& SpriteTypes::isWindow(t.spriteDef->type))
							return true;
					}
					return false;
				};
				if (_rules.forbidAdjacentWindows && (hasWindowAt(wx - 1, cy) || hasWindowAt(wx + 1, cy)))
					continue;
				setTile(result, wx, cy, pick(_windows, rng));
				occupied.insert(keyXY(wx, cy));
				if (_rules.oneWindowPerCave)
					break;
			}
		}
	}

	std::vector<std::pair<int, int>> starts;
	const int waterLimit = maxWalkableRow() + 1;
	const int upperHalf = static_cast<int>(_height) / 2;
	int bestScore = -1;
	std::vector<std::pair<int, int>> bestStarts;
	for (const auto& a : airCells) {
		if (a.second < 1)
			continue;
		if (a.second >= waterLimit)
			continue;
		if (occupied.count(keyXY(a.first, a.second)))
			continue;

		int openness = 0;
		for (int d = 0; d < 4; ++d) {
			const int nx = a.first + DIR[d][0];
			const int ny = a.second + DIR[d][1];
			if (nx < 0 || ny < 0 || nx >= static_cast<int>(_width) || ny >= static_cast<int>(_height))
				continue;
			if (at(grid, nx, ny) == Cell::Air)
				++openness;
		}
		int score = openness * 10;
		if (a.second < upperHalf)
			score += 5;
		// Prefer cells with air above (not tucked under a ceiling)
		if (at(grid, a.first, a.second - 1) == Cell::Air)
			score += 3;

		if (score > bestScore) {
			bestScore = score;
			bestStarts.clear();
			bestStarts.push_back(a);
		} else if (score == bestScore) {
			bestStarts.push_back(a);
		}
		starts.push_back(a);
	}
	const std::vector<std::pair<int, int>>& pool = bestStarts.empty() ? starts : bestStarts;
	if (!pool.empty()) {
		const auto& s = pool[randRange(rng, static_cast<int>(pool.size()))];
		result.startPositions.push_back({ string::toString(s.first), string::toString(s.second) });
	}

	auto trySurfaceEmitters = [&] (const EntityType& type, int chance, int maxCount, bool guaranteeOne) {
		if (chance <= 0 || maxCount <= 0)
			return 0;
		const float entW = type.width;
		const float entH = type.height;
		const int ew = std::max(1, static_cast<int>(std::ceil(static_cast<double>(entW) - 1e-3)));
		const int eh = std::max(1, static_cast<int>(std::ceil(static_cast<double>(entH) - 1e-3)));

		auto canPlace = [&] (int gx, int gy) -> bool {
			// gy = ground row; emitter origin is top-left of the entity footprint (matches hand maps).
			if (gy < eh || gx < 0 || gx + ew > static_cast<int>(_width))
				return false;
			if (!isFlatEmitterSupport(at(grid, gx, gy)))
				return false;
			for (int dx = 0; dx < ew; ++dx) {
				if (!isFlatEmitterSupport(at(grid, gx + dx, gy)))
					return false;
				if (occupied.count(keyXY(gx + dx, gy)))
					return false;
			}
			const int ey = gy - eh;
			for (int dx = 0; dx < ew; ++dx) {
				for (int dy = 0; dy < eh; ++dy) {
					if (at(grid, gx + dx, ey + dy) != Cell::Air)
						return false;
					if (occupied.count(keyXY(gx + dx, ey + dy)))
						return false;
				}
			}
			return true;
		};
		auto placeAt = [&] (int gx, int gy) {
			const int ey = gy - eh;
			result.emitters.emplace_back(static_cast<gridCoord>(gx), static_cast<gridCoord>(ey), type, 1, 0, "");
			for (int dx = 0; dx < ew; ++dx) {
				occupied.insert(keyXY(gx + dx, gy));
				for (int dy = 0; dy < eh; ++dy)
					occupied.insert(keyXY(gx + dx, ey + dy));
			}
		};

		int placed = 0;
		for (const auto& g : surfaceCells) {
			if (placed >= maxCount)
				break;
			if (randRange(rng, chance) != 0)
				continue;
			if (!canPlace(g.first, g.second))
				continue;
			placeAt(g.first, g.second);
			++placed;
		}
		if (placed == 0 && maxCount > 0 && guaranteeOne) {
			for (const auto& g : surfaceCells) {
				if (!canPlace(g.first, g.second))
					continue;
				placeAt(g.first, g.second);
				++placed;
				break;
			}
		}
		return placed;
	};
	trySurfaceEmitters(EntityTypes::STONE, _rules.stoneChance, 2, false);
	trySurfaceEmitters(EntityTypes::TREE, _rules.treeChance, std::max(1, _rules.maxTrees),
			_rules.minTreeEmitters > 0);
	trySurfaceEmitters(EntityTypes::NPC_WALKING, _rules.walkingChance, 1, false);
	if (placedTarget && _kit.packageEntity != nullptr)
		trySurfaceEmitters(*_kit.packageEntity, _rules.packageChance, 1, false);
}

RandomMapGenerator::Result RandomMapGenerator::generate (unsigned int seed)
{
	Result result;
	result.title = "random " + _theme->name + " (seed " + string::toString(seed) + ")";
	result.settings[msn::WIDTH] = string::toString(_width);
	result.settings[msn::HEIGHT] = string::toString(_height);
	result.settings[msn::THEME] = _theme->name;
	result.settings[msn::WATER_HEIGHT] = string::toString(_waterHeight);
	result.settings[msn::WATER_CHANGE] = "0.0";
	result.settings[msn::GRAVITY] = string::toString(msdv::GRAVITY);
	result.settings[msn::POINTS] = string::toString(msdv::POINTS);
	result.settings[msn::REFERENCETIME] = string::toString(msdv::REFERENCETIME);
	result.settings[msn::PACKAGE_TRANSFER_COUNT] = string::toString(_kit.packageTransferCount);
	result.settings[msn::NPC_TRANSFER_COUNT] = string::toString(_kit.npcTransferCount);
	result.settings[msn::FLYING_NPC] = "false";
	result.settings[msn::FISH_NPC] = "false";
	result.settings[msn::WIND] = "0.0";
	result.settings["seed"] = string::toString(seed);

	if (_rocksFull.empty() || _backgrounds.empty()) {
		result.failureReason = "missing theme sprites for " + _theme->name;
		Log::error(LOG_GAMEIMPL, "mapgen: %s", result.failureReason.c_str());
		return result;
	}

	unsigned int rng = seed ? seed : 1u;
	const int maxAttempts = std::max(1, _rules.maxGenerateAttempts);
	const int mapCells = static_cast<int>(_width * _height);
	for (int attempt = 0; attempt < maxAttempts; ++attempt) {
		std::vector<uint16_t> domain(_width * _height, MASK_ALL);
		std::vector<Cell> grid;
		unsigned int attemptSeed = rng + static_cast<unsigned int>(attempt) * 9973u;
		if (!collapse(domain, grid, attemptSeed))
			continue;
		if (!isAirConnected(grid))
			continue;

		int surfaces = 0;
		int airCells = 0;
		for (Cell c : grid) {
			if (isWalkableSurface(c))
				++surfaces;
			if (c == Cell::Air)
				++airCells;
		}
		if (!_rules.acceptsGrid(surfaces, airCells, mapCells))
			continue;

		instantiate(grid, result, attemptSeed);
		if (_rules.requirePlayerStart && result.startPositions.empty())
			continue;
		if (_rules.requireCaveIfAvailable && result.caves.empty() && !_caves.empty())
			continue;
		if (_kit.packageTargetRequired && result.tiles.end() == std::find_if(result.tiles.begin(), result.tiles.end(),
				[] (const MapTileDefinition& t) {
					return t.spriteDef && SpriteTypes::isPackageTarget(t.spriteDef->type);
				}))
			continue;

		const MapMetrics metrics = MapValidator().evaluate(static_cast<int>(_width), static_cast<int>(_height),
				result.tiles, result.caves, result.emitters, result.startPositions, _rules.minCaveSeparation,
				_rules.minCavePackageAirSeparation, _rules.minPlatformLength, _rules.minSolidComponentSize);
		if (!_rules.accepts(metrics, static_cast<int>(_width), static_cast<int>(_height), _waterHeight)) {
			Log::debug(LOG_GAMEIMPL, "random map reject seed=%u attempt %i: %s score=%.1f rows=%i walk=%i wall=%i trees=%i",
					seed, attempt + 1, metrics.failureReason.c_str(), metrics.totalScore,
					metrics.platformRows, metrics.walkableCells, metrics.colliderCells,
					metrics.treeEmitterCount);
			result.tiles.clear();
			result.caves.clear();
			result.emitters.clear();
			result.startPositions.clear();
			continue;
		}

		result.success = true;
		result.failureReason.clear();
		Log::info(LOG_GAMEIMPL, "random map generated %ux%u map seed=%u (attempt %i, surfaces=%i caves=%i score=%.1f water=%.2f)",
				_width, _height, seed, attempt + 1, surfaces, (int)result.caves.size(), metrics.totalScore,
				_waterHeight);
		return result;
	}

	result.failureReason = "no valid layout after " + string::toString(maxAttempts)
			+ " attempts (theme=" + _theme->name + ", seed=" + string::toString(seed) + ")";
	Log::error(LOG_GAMEIMPL, "mapgen: %s", result.failureReason.c_str());
	return result;
}

}
