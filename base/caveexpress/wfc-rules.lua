-- CaveExpress WFC generator knobs (loaded by WfcRules::loadFromLua).
function getWfcRules()
	return {
		caves = {
			target = 3,
			minSeparation = 3,
			npcChance = 0.75
		},
		windows = {
			enabled = true,
			onePerCave = true,
			forbidAdjacentWindows = true
		},
		platforms = {
			bandMin = 2,
			bandMax = 3,
			minVerticalGap = 5,
			lengthMin = 3,
			lengthMax = 5,
			gapMin = 2,
			gapMax = 4,
			floatingChance = 0.2
		},
		bridges = {
			enabled = true,
			maxGap = 3
		},
		weights = {
			air = 60,
			rock = 40,
			ground = 8,
			ledge = 3,
			undercut = 4,
			shim = 2
		},
		packageTarget = {
			required = true,
			angles = { 0 },
			requireAirOppositeSupport = true,
			sidesMustBeWalkable = true
		},
		emitters = {
			stoneChance = 5,
			treeChance = 6,
			walkingChance = 8,
			packageChance = 2
		},
		decor = {
			caveArtChance = 6
		}
	}
end
