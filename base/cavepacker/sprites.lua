sprites = {

-- Player

	["player-idle"] = {
		frames = {
			{}, --back
			{ "player", }, --middle
			{}, --front
		}
	},

-- Package

	["tile-target-01"] = {
		rotateable = 1,
		theme = "rock",
	},

	["item-package-idle"] = {
		rotateable = 1,
		theme = "rock",
	},

-- Rock Tiles

	["tile-background-01"] = { type = "ground", theme = "rock", },
	["tile-background-02"] = { type = "ground", theme = "rock", },
	["tile-background-03"] = { type = "ground", theme = "rock", },
	["tile-background-04"] = { type = "ground", theme = "rock", },
	["tile-background-big-01"] = { type = "ground", width = 2, height = 2, theme = "rock", },

	["tile-rock-01"] = { type = "solid", theme = "rock", },
	["tile-rock-02"] = { type = "solid", theme = "rock", },
	["tile-rock-03"] = { type = "solid", theme = "rock", },
	["tile-rock-big-01"] = { type = "solid", width = 2, height = 2, theme = "rock", },
}
