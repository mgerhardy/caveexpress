#include "MapValidator.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "common/String.h"
#include "common/vec2.h"
#include <algorithm>
#include <cmath>
#include <queue>

namespace caveexpress {

MapValidator::CellKind MapValidator::classifyTile (const SpriteType& type) const
{
	if (SpriteTypes::isAnyGround(type) || SpriteTypes::isBridge(type))
		return CellKind::Walkable;
	if (SpriteTypes::isSolid(type) || SpriteTypes::isPackageTarget(type))
		return CellKind::Collider;
	if (SpriteTypes::isBackground(type) || SpriteTypes::isWindow(type) || SpriteTypes::isCave(type)
			|| SpriteTypes::isLiane(type))
		return CellKind::FlyableDecor;
	return CellKind::FlyableDecor;
}

void MapValidator::paintSprite (Grid& grid, const SpriteDefPtr& def, int x, int y, CellKind kind) const
{
	if (!def)
		return;
	const int w = std::max(1, static_cast<int>(std::ceil(def->width - 0.001f)));
	const int h = std::max(1, static_cast<int>(std::ceil(def->height - 0.001f)));
	for (int dy = 0; dy < h; ++dy) {
		for (int dx = 0; dx < w; ++dx) {
			const int cx = x + dx;
			const int cy = y + dy;
			if (!grid.inBounds(cx, cy))
				continue;
			const int i = grid.idx(cx, cy);
			// Solids overwrite decor; walkable preferred over generic collider if both claim
			if (kind == CellKind::Walkable) {
				grid.kind[i] = CellKind::Walkable;
			} else if (kind == CellKind::Collider) {
				if (grid.kind[i] != CellKind::Walkable)
					grid.kind[i] = CellKind::Collider;
			} else if (grid.kind[i] == CellKind::Empty) {
				grid.kind[i] = kind;
			}

			if (SpriteTypes::isWindow(def->type))
				grid.isWindow[i] = 1;
			if (SpriteTypes::isCave(def->type))
				grid.isCave[i] = 1;
			if (SpriteTypes::isPackageTarget(def->type))
				grid.isPackageTarget[i] = 1;
			if (SpriteTypes::isBridge(def->type))
				grid.isBridge[i] = 1;
			if (SpriteTypes::isBackground(def->type))
				grid.isBackground[i] = 1;
		}
	}
}

void MapValidator::floodFlyable (const Grid& grid, int sx, int sy, std::vector<uint8_t>& reached) const
{
	reached.assign(grid.width * grid.height, 0);
	if (!grid.flyable(sx, sy))
		return;
	std::queue<int> q;
	q.push(grid.idx(sx, sy));
	reached[grid.idx(sx, sy)] = 1;
	static const int DIR[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
	while (!q.empty()) {
		const int i = q.front();
		q.pop();
		const int x = i % grid.width;
		const int y = i / grid.width;
		for (const auto& d : DIR) {
			const int nx = x + d[0];
			const int ny = y + d[1];
			if (!grid.flyable(nx, ny))
				continue;
			const int ni = grid.idx(nx, ny);
			if (reached[ni])
				continue;
			reached[ni] = 1;
			q.push(ni);
		}
	}
}

int MapValidator::airPathDistance (const Grid& grid, int sx, int sy, int gx, int gy) const
{
	if (!grid.flyable(sx, sy) || !grid.flyable(gx, gy))
		return -1;
	if (sx == gx && sy == gy)
		return 0;

	const int n = grid.width * grid.height;
	std::vector<int> dist(n, -1);
	std::queue<int> q;
	const int start = grid.idx(sx, sy);
	const int goal = grid.idx(gx, gy);
	dist[start] = 0;
	q.push(start);
	static const int DIR[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
	while (!q.empty()) {
		const int i = q.front();
		q.pop();
		if (i == goal)
			return dist[i];
		const int x = i % grid.width;
		const int y = i / grid.width;
		for (const auto& d : DIR) {
			const int nx = x + d[0];
			const int ny = y + d[1];
			if (!grid.flyable(nx, ny))
				continue;
			const int ni = grid.idx(nx, ny);
			if (dist[ni] >= 0)
				continue;
			dist[ni] = dist[i] + 1;
			q.push(ni);
		}
	}
	return -1;
}

MapMetrics MapValidator::evaluate (int width, int height,
		const std::vector<MapTileDefinition>& tiles,
		const std::vector<CaveTileDefinition>& caves,
		const std::vector<EmitterDefinition>& emitters,
		const IMap::StartPositions& starts,
		int minCaveSeparation,
		int minCavePackageAirSeparation,
		int minPlatformLength,
		int minSolidComponentSize) const
{
	MapMetrics m;
	m.width = width;
	m.height = height;
	if (width <= 0 || height <= 0) {
		m.failureReason = "invalid dimensions";
		return m;
	}

	Grid grid;
	grid.width = width;
	grid.height = height;
	const int n = width * height;
	grid.kind.assign(n, CellKind::Empty);
	grid.isWindow.assign(n, 0);
	grid.isCave.assign(n, 0);
	grid.isPackageTarget.assign(n, 0);
	grid.isBridge.assign(n, 0);
	grid.isBackground.assign(n, 0);

	for (const MapTileDefinition& tile : tiles) {
		if (!tile.spriteDef)
			continue;
		paintSprite(grid, tile.spriteDef, static_cast<int>(tile.x), static_cast<int>(tile.y),
				classifyTile(tile.spriteDef->type));
	}
	for (const CaveTileDefinition& cave : caves) {
		if (!cave.spriteDef)
			continue;
		const int cx = static_cast<int>(cave.x);
		const int cy = static_cast<int>(cave.y);
		paintSprite(grid, cave.spriteDef, cx, cy, CellKind::FlyableDecor);
		if (grid.inBounds(cx, cy))
			grid.isCave[grid.idx(cx, cy)] = 1;
	}

	std::vector<std::pair<int, int>> cavePositions;
	std::vector<std::pair<int, int>> packageTargets;
	std::vector<std::pair<int, int>> packageEmitters;
	std::vector<std::pair<int, int>> windows;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const int i = grid.idx(x, y);
			if (grid.isCave[i])
				cavePositions.emplace_back(x, y);
			if (grid.isPackageTarget[i])
				packageTargets.emplace_back(x, y);
			if (grid.isWindow[i])
				windows.emplace_back(x, y);
			if (grid.kind[i] == CellKind::Walkable)
				++m.walkableCells;
			else if (grid.kind[i] == CellKind::Collider)
				++m.colliderCells;
			if (grid.solid(x, y))
				++m.solidCells;
			else
				++m.flyableCells;
		}
	}
	// Caves list may be authoritative even if sprite paint missed
	if (cavePositions.empty()) {
		for (const CaveTileDefinition& cave : caves)
			cavePositions.emplace_back(static_cast<int>(cave.x), static_cast<int>(cave.y));
	}
	m.caveCount = static_cast<int>(cavePositions.size());
	m.packageTargetCount = static_cast<int>(packageTargets.size());
	m.windowCount = static_cast<int>(windows.size());

	for (const EmitterDefinition& e : emitters) {
		if (!e.type)
			continue;
		if (EntityTypes::isPackage(*e.type)) {
			packageEmitters.emplace_back(static_cast<int>(std::floor(e.x + EPSILON)),
					static_cast<int>(std::floor(e.y + EPSILON)));
			++m.packageEmitterCount;
		} else if (EntityTypes::isPackageTarget(*e.type)) {
			packageTargets.emplace_back(static_cast<int>(std::floor(e.x + EPSILON)),
					static_cast<int>(std::floor(e.y + EPSILON)));
			++m.packageTargetCount;
		} else if (EntityTypes::isTree(*e.type)) {
			++m.treeEmitterCount;
		}
	}

	int startX = -1;
	int startY = -1;
	if (!starts.empty()) {
		startX = string::toInt(starts[0]._x);
		startY = string::toInt(starts[0]._y);
	}
	if (startX < 0 || startY < 0 || !grid.flyable(startX, startY)) {
		// Fallback: first flyable cell
		for (int y = 0; y < height && startX < 0; ++y) {
			for (int x = 0; x < width; ++x) {
				if (grid.flyable(x, y)) {
					startX = x;
					startY = y;
					break;
				}
			}
		}
	}
	if (startX < 0) {
		m.failureReason = "no flyable start";
		return m;
	}

	std::vector<uint8_t> reached;
	floodFlyable(grid, startX, startY, reached);

	for (int i = 0; i < n; ++i) {
		if (grid.kind[i] != CellKind::Empty && grid.kind[i] != CellKind::FlyableDecor)
			continue;
		if (reached[i])
			++m.flyableReachable;
		else
			++m.unreachableFlyable;
	}
	m.flyableReachabilityRatio = m.flyableCells > 0
			? static_cast<float>(m.flyableReachable) / static_cast<float>(m.flyableCells)
			: 0.0f;

	auto poiReachable = [&] (int x, int y) {
		static const int DIR[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
		if (grid.flyable(x, y) && reached[grid.idx(x, y)])
			return true;
		for (const auto& d : DIR) {
			const int nx = x + d[0];
			const int ny = y + d[1];
			if (grid.flyable(nx, ny) && reached[grid.idx(nx, ny)])
				return true;
		}
		return false;
	};

	for (const auto& c : cavePositions) {
		if (poiReachable(c.first, c.second))
			++m.cavesReachable;
	}
	for (const auto& t : packageTargets) {
		// Delivery from air above the target (player approach cell).
		const int ax = t.first;
		const int ay = t.second - 1;
		if ((grid.inBounds(ax, ay) && grid.flyable(ax, ay) && reached[grid.idx(ax, ay)]) || poiReachable(t.first, t.second))
			++m.packageTargetsReachable;
	}

	// Package emitters reach same component as some target delivery cell
	std::vector<uint8_t> targetAir(n, 0);
	for (const auto& t : packageTargets) {
		const int ax = t.first;
		const int ay = t.second - 1;
		if (grid.flyable(ax, ay))
			targetAir[grid.idx(ax, ay)] = 1;
		if (grid.flyable(t.first, t.second))
			targetAir[grid.idx(t.first, t.second)] = 1;
	}
	for (const auto& p : packageEmitters) {
		bool ok = false;
		// Packages rest near a surface - probe the cell and flyable neighbors for shared airspace with targets.
		static const int OFF[5][2] = { { 0, 0 }, { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 } };
		bool anyTargetInComponent = false;
		for (int i = 0; i < n; ++i) {
			if (targetAir[i] && reached[i]) {
				anyTargetInComponent = true;
				break;
			}
		}
		if (anyTargetInComponent) {
			for (const auto& o : OFF) {
				const int nx = p.first + o[0];
				const int ny = p.second + o[1];
				if (grid.flyable(nx, ny) && reached[grid.idx(nx, ny)]) {
					ok = true;
					break;
				}
			}
			// Also accept if the package sits on a solid whose above-air is reachable
			if (!ok && grid.inBounds(p.first, p.second) && grid.solid(p.first, p.second)) {
				const int ax = p.first;
				const int ay = p.second - 1;
				if (grid.flyable(ax, ay) && reached[grid.idx(ax, ay)])
					ok = true;
			}
		}
		if (ok)
			++m.packagesReachableToTarget;
	}
	if (packageEmitters.empty() && !packageTargets.empty() && m.packageTargetsReachable > 0)
		m.packagesReachableToTarget = 1;

	// Soft metrics
	for (int y = 1; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (!grid.solid(x, y))
				continue;
			if (!grid.flyable(x, y - 1))
				continue;
			++m.solidWithAirAbove;
			const CellKind k = grid.kind[grid.idx(x, y)];
			if (k != CellKind::Walkable)
				++m.exposedRockTops;
		}
	}
	m.walkableSurfaceRatio = m.solidWithAirAbove > 0
			? 1.0f - static_cast<float>(m.exposedRockTops) / static_cast<float>(m.solidWithAirAbove)
			: 1.0f;
	m.exposedRockTopRatio = m.solidWithAirAbove > 0
			? static_cast<float>(m.exposedRockTops) / static_cast<float>(m.solidWithAirAbove)
			: 0.0f;

	static const int N4[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (grid.kind[grid.idx(x, y)] != CellKind::Collider)
				continue;
			bool hasNeighbor = false;
			for (const auto& d : N4) {
				const int nx = x + d[0];
				const int ny = y + d[1];
				if (!grid.inBounds(nx, ny) || grid.solid(nx, ny)) {
					hasNeighbor = true;
					break;
				}
			}
			if (!hasNeighbor)
				++m.orphanColliders;
		}
	}
	m.orphanColliderRatio = m.colliderCells > 0
			? static_cast<float>(m.orphanColliders) / static_cast<float>(m.colliderCells)
			: 0.0f;

	m.airOpenness = n > 0 ? static_cast<float>(m.flyableCells) / static_cast<float>(n) : 0.0f;

	// Platform run lengths on non-bridge walkable rows. Package targets sit in the
	// walkable niche and should not split an otherwise coherent platform.
	std::vector<int> runs;
	for (int y = 0; y < height; ++y) {
		int run = 0;
		for (int x = 0; x < width; ++x) {
			const int i = grid.idx(x, y);
			const bool plat = (grid.kind[i] == CellKind::Walkable && !grid.isBridge[i]) || grid.isPackageTarget[i];
			if (plat) {
				++run;
			} else if (run > 0) {
				runs.push_back(run);
				if (run < minPlatformLength)
					++m.shortPlatformRuns;
				run = 0;
			}
		}
		if (run > 0) {
			runs.push_back(run);
			if (run < minPlatformLength)
				++m.shortPlatformRuns;
		}
	}
	if (!runs.empty()) {
		int sum = 0;
		for (int r : runs)
			sum += r;
		m.meanPlatformLength = static_cast<float>(sum) / static_cast<float>(runs.size());
	}

	// Distinct rows that host a real platform (non-bridge walkable / package niche).
	for (int y = 0; y < height; ++y) {
		int run = 0;
		bool rowCounts = false;
		for (int x = 0; x < width; ++x) {
			const int i = grid.idx(x, y);
			const bool plat = (grid.kind[i] == CellKind::Walkable && !grid.isBridge[i]) || grid.isPackageTarget[i];
			if (plat) {
				++run;
				if (run >= minPlatformLength)
					rowCounts = true;
			} else {
				run = 0;
			}
		}
		if (rowCounts)
			++m.platformRows;
	}

	// Isolated non-bridge walkable tiles (no walkable/package-target orthogonal neighbor).
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const int i = grid.idx(x, y);
			if (grid.kind[i] != CellKind::Walkable || grid.isBridge[i])
				continue;
			int walkNeighbors = 0;
			for (const auto& d : N4) {
				const int nx = x + d[0];
				const int ny = y + d[1];
				if (!grid.inBounds(nx, ny))
					continue;
				const int ni = grid.idx(nx, ny);
				if ((grid.kind[ni] == CellKind::Walkable) || grid.isPackageTarget[ni])
					++walkNeighbors;
			}
			if (walkNeighbors == 0)
				++m.isolatedWalkables;
		}
	}

	// Small solid components (4-connected) not counting the bottom water bed as "good structure".
	{
		std::vector<uint8_t> seen(n, 0);
		std::vector<int> stack;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				const int start = grid.idx(x, y);
				if (seen[start] || !grid.solid(x, y))
					continue;
				stack.clear();
				stack.push_back(start);
				seen[start] = 1;
				int size = 0;
				bool touchesBottom = false;
				while (!stack.empty()) {
					const int i = stack.back();
					stack.pop_back();
					++size;
					const int cx = i % width;
					const int cy = i / width;
					if (cy >= height - 2)
						touchesBottom = true;
					for (const auto& d : N4) {
						const int nx = cx + d[0];
						const int ny = cy + d[1];
						if (!grid.inBounds(nx, ny) || !grid.solid(nx, ny))
							continue;
						const int ni = grid.idx(nx, ny);
						if (seen[ni])
							continue;
						seen[ni] = 1;
						stack.push_back(ni);
					}
				}
				if (!touchesBottom && size < minSolidComponentSize)
					++m.smallSolidComponents;
			}
		}
	}

	// Cave separation
	m.minCaveSeparation = m.caveCount <= 1 ? 99.0f : 99.0f;
	if (m.caveCount >= 2) {
		int best = 999;
		for (size_t i = 0; i < cavePositions.size(); ++i) {
			for (size_t j = i + 1; j < cavePositions.size(); ++j) {
				const int d = std::max(std::abs(cavePositions[i].first - cavePositions[j].first),
						std::abs(cavePositions[i].second - cavePositions[j].second));
				best = std::min(best, d);
				if (d < minCaveSeparation)
					++m.cavesTooClose;
			}
		}
		m.minCaveSeparation = static_cast<float>(best);
	}

	// Window-window adjacency
	for (const auto& w : windows) {
		if (grid.inBounds(w.first + 1, w.second) && grid.isWindow[grid.idx(w.first + 1, w.second)])
			++m.windowWindowAdjacencies;
		if (grid.inBounds(w.first, w.second + 1) && grid.isWindow[grid.idx(w.first, w.second + 1)])
			++m.windowWindowAdjacencies;
	}

	// Package target niche for delivery from air above the target (player approach
	// cell): walkable L/R, solid below, air above.
	for (const auto& t : packageTargets) {
		const int x = t.first;
		const int y = t.second;
		const bool leftOk = grid.solid(x - 1, y);
		const bool rightOk = grid.solid(x + 1, y);
		const bool belowOk = grid.solid(x, y + 1);
		const bool aboveOk = grid.flyable(x, y - 1);
		const bool leftWalk = grid.inBounds(x - 1, y) && grid.kind[grid.idx(x - 1, y)] == CellKind::Walkable;
		const bool rightWalk = grid.inBounds(x + 1, y) && grid.kind[grid.idx(x + 1, y)] == CellKind::Walkable;
		if (!(leftOk && rightOk && belowOk && aboveOk && leftWalk && rightWalk))
			++m.packageTargetsWithBadNiche;
	}

	// Cave vs package-target layout: no cave stacked above a target, and a minimum
	// flyable air-path length (walls/gaps that force a detour count as farther).
	m.minCavePackageAirPath = (m.caveCount == 0 || m.packageTargetCount == 0) ? 99.0f : 99.0f;
	if (m.caveCount > 0 && m.packageTargetCount > 0) {
		int bestAir = 999;
		for (const auto& c : cavePositions) {
			for (const auto& t : packageTargets) {
				if (c.first == t.first && c.second < t.second)
					++m.cavesAbovePackageTarget;

				int caveX = c.first;
				int caveY = c.second;
				if (!grid.flyable(caveX, caveY)) {
					// Fall back to a flyable neighbor of the cave entry
					static const int OFF[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
					bool found = false;
					for (const auto& o : OFF) {
						if (grid.flyable(c.first + o[0], c.second + o[1])) {
							caveX = c.first + o[0];
							caveY = c.second + o[1];
							found = true;
							break;
						}
					}
					if (!found)
						continue;
				}

				int targetAirX = t.first;
				int targetAirY = t.second - 1;
				if (!grid.flyable(targetAirX, targetAirY)) {
					static const int OFF[4][2] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
					bool found = false;
					for (const auto& o : OFF) {
						if (grid.flyable(t.first + o[0], t.second + o[1])) {
							targetAirX = t.first + o[0];
							targetAirY = t.second + o[1];
							found = true;
							break;
						}
					}
					if (!found)
						continue;
				}

				const int dist = airPathDistance(grid, caveX, caveY, targetAirX, targetAirY);
				if (dist < 0)
					continue;
				bestAir = std::min(bestAir, dist);
				if (dist < minCavePackageAirSeparation)
					++m.cavePackageAirTooClose;
			}
		}
		m.minCavePackageAirPath = bestAir >= 999 ? -1.0f : static_cast<float>(bestAir);
	}

	// Bridges should have background on the same cell (or open air below as a weaker proxy)
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (!grid.isBridge[grid.idx(x, y)])
				continue;
			const bool sameCellBg = grid.isBackground[grid.idx(x, y)] != 0;
			const bool airBelow = y + 1 < height && !grid.solid(x, y + 1);
			if (!sameCellBg && !airBelow)
				++m.bridgesWithoutBackground;
		}
	}

	// Hard validity (POI reachability). Full flyable coverage is a soft metric that
	// mapgen generation enforces separately — hand maps often paint backgrounds into
	// enclosed rock niches for visuals.
	m.valid = true;
	if (m.caveCount > 0 && m.cavesReachable < m.caveCount) {
		m.valid = false;
		m.failureReason = "unreachable cave";
	}
	if (m.packageTargetCount > 0 && m.packageTargetsReachable < m.packageTargetCount) {
		m.valid = false;
		if (m.failureReason.empty())
			m.failureReason = "unreachable package target";
	}
	if (m.packageEmitterCount > 0 && m.packageTargetCount > 0 && m.packagesReachableToTarget == 0) {
		m.valid = false;
		if (m.failureReason.empty())
			m.failureReason = "package cannot reach target airspace";
	}
	if (m.failureReason.empty() && m.unreachableFlyable > 0)
		m.failureReason = "unreachable flyable";

	// Composite score 0-100
	float score = 0.0f;
	score += 20.0f * m.walkableSurfaceRatio;
	score += 12.0f * (1.0f - std::min(1.0f, m.exposedRockTopRatio));
	score += 12.0f * (1.0f - std::min(1.0f, m.orphanColliderRatio * 5.0f));
	score += 8.0f * std::min(1.0f, m.airOpenness / 0.45f);
	score += 5.0f * m.flyableReachabilityRatio;
	score += 8.0f * (m.caveCount == 0 ? 1.0f :
			static_cast<float>(m.cavesReachable) / static_cast<float>(m.caveCount));
	score += 8.0f * (m.packageTargetCount == 0 ? 1.0f :
			static_cast<float>(m.packageTargetsReachable) / static_cast<float>(m.packageTargetCount));
	score += 5.0f * (m.windowWindowAdjacencies == 0 ? 1.0f : 0.0f);
	score += 5.0f * (m.cavesTooClose == 0 ? 1.0f : 0.0f);
	score += 5.0f * (m.cavesAbovePackageTarget == 0 ? 1.0f : 0.0f);
	score += 5.0f * (m.cavePackageAirTooClose == 0 ? 1.0f : 0.0f);
	score += 4.0f * (m.shortPlatformRuns == 0 ? 1.0f : 0.0f);
	score += 4.0f * (m.smallSolidComponents == 0 ? 1.0f : 0.0f);
	score += 3.0f * (m.isolatedWalkables == 0 ? 1.0f : 0.0f);
	score += 4.0f * std::min(1.0f, static_cast<float>(m.platformRows) / 3.0f);
	score += 3.0f * (m.treeEmitterCount > 0 ? 1.0f : 0.0f);
	score += 4.0f * (m.packageTargetCount == 0 ? 1.0f :
			1.0f - static_cast<float>(m.packageTargetsWithBadNiche) / static_cast<float>(m.packageTargetCount));
	if (!m.valid)
		score *= 0.35f;
	m.totalScore = std::max(0.0f, std::min(100.0f, score));
	return m;
}

}
