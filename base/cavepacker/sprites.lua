-- Point lights sampled by sprites that have a normal map (*_n).
-- CavePacker positions are UPPER_LEFT (tile origin), so offsets are in tiles
-- from the top-left of the cell (Y increases downward).
local LIGHT_TARGET = {
	radius = 2.5,
	intensity = 0.65,
	color = { 1.0, 0.82, 0.22 },
	offsetx = 0.5,
	offsety = 0.5,
	falloff = 2.0,
}
local LIGHT_TORCH = {
	radius = 4.0,
	intensity = 0.9,
	color = { 1.0, 0.72, 0.32 },
	offsetx = 0.5,
	offsety = 0.85,
	falloff = 2.0,
}

sprites = {
	["player"] = {
		fps = 8,
	},
	["target"] = { type = "target", fps = 3, light = LIGHT_TARGET, },
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
	["tile-rock-05"] = { type = "solid", placement = "down", light = LIGHT_TORCH, }, -- torch faces down / inward
	["tile-rock-06"] = { type = "solid", placement = "right", light = LIGHT_TORCH, }, -- torch faces right / inward
}
