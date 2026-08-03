-- CaveExpress random map generator knobs (loaded by RandomMapRules::loadFromLua).
--
-- weights.air / weights.rock dominate collapse after border+platform seeds.
-- ground / ledge / undercut / shim rarely appear in observe for unseeded cells;
-- slopes are placed only in decorateSlopes (not observe).
function getRandomMapRules()
	return {
		caves = {
			target = 3,
			minSeparation = 3,
			npcChance = 0.75,
			npcDelay = 5000,
			npcTypes = { "npc-man", "npc-woman", "npc-grandpa" }
		},
		windows = {
			enabled = true,
			onePerCave = true,
			forbidAdjacentWindows = true
		},
		platforms = {
			bandMin = 3,
			bandMax = 4,
			minVerticalGap = 3,
			lengthMin = 4,
			lengthMax = 7,
			gapMin = 2,
			gapMax = 4,
			floatingChance = 0.28
		},
		bridges = {
			enabled = true,
			maxGap = 3
		},
		weights = {
			air = 68,
			rock = 32,
			ground = 8,
			ledge = 3,
			undercut = 4,
			shim = 2
		},
		packageTarget = {
			-- Required only when the theme has package-target sprites (rock/ice).
			-- Jungle/desert automatically use NPC-transfer mode.
			required = true,
			transferCount = 3,
			requireAirOppositeSupport = true,
			sidesMustBeWalkable = true,
			minCaveAirSeparation = 4
		},
		cleanup = {
			minPlatformLength = 3,
			minSolidComponentSize = 4,
			minPlatformRows = 3,
			minWalkableCells = 18,
			minColliderCells = 20,
			minTreeEmitters = 1,
			minTotalScore = 40,
			maxExposedRockTopRatio = 0.55,
			maxOrphanColliderRatio = 0.20,
			maxGenerateAttempts = 96,
			minSurfaceCells = 6,
			minAirPercent = 25,
			requirePlayerStart = true,
			requireCaveIfAvailable = true
		},
		emitters = {
			stoneChance = 4,
			treeChance = 3,
			walkingChance = 6,
			packageChance = 2,
			maxTrees = 3
		},
		decor = {
			-- 1/N chance to pick from cave-art backgrounds (id contains "cave-art")
			caveArtChance = 6
		}
	}
end
