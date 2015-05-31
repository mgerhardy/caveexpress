sprites = {

-- Player

	["player"] = {
		frames = {
			{}, --back
			{ "player", }, --middle
			{}, --front
		}
	},

	["target"] = {
		type = "target",
		rotateable = 1,
		theme = "rock",
		fps = 1,
		frames = {
			{ "tile-target-01-middle-01", }, --back
			{}, --middle
			{}, --front
		}
	},

-- Package

	["package"] = {
		type = "package",
		rotateable = 1,
		theme = "rock",
		frames = {
			{}, --back
			{}, --middle
			{ "item-package-idle-middle-01", }, --front
		}
	},
	["package-delivered"] = {
		type = "package",
		rotateable = 1,
		theme = "rock",
		frames = {
			{}, --back
			{}, --middle
			{ "item-package-delivered-middle-01", }, --front
		}
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
