sprites = {
	["player"] = {
		fps = 8,
	},
	["target"] = { type = "target", fps = 3, },
	["package"] = { type = "package", },
	["package-delivered"] = { type = "package", },
	["package-deadlock"] = { type = "package", },
	["deadlock"] = {},
	["tile-background-01"] = { type = "ground", },
	["tile-background-02"] = { type = "ground", },
	["tile-background-03"] = { type = "ground", },
	["tile-background-04"] = { type = "ground", },
	-- placement: any|left|right|top|down|full - each rock art has one fixed orientation
	["tile-rock-01"] = { type = "solid", placement = "any", },
	["tile-rock-02"] = { type = "solid", placement = "any", },
	["tile-rock-03"] = { type = "solid", placement = "any", },
	["tile-rock-04"] = { type = "solid", placement = "down", }, -- cave opening faces down / inward
	["tile-rock-05"] = { type = "solid", placement = "down", }, -- torch faces down / inward
}
