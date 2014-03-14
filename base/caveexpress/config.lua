defaultwidth = -1
defaultheight = -1
defaultfullscreen = false
defaultpersister = "sqlite"
defaultfrontend = "opengl"
defaultsoundengine = "sdl"
defaultnetwork = true
defaultshowcursor = true
defaultjoystick = true
defaulttexturesize = "auto"
defaultreferencetimefactor = 1.0
defaultdamagethreshold = 3.0
defaultnpcflyingspeed = 4.0
defaultparticles = 100

settings = {
	width = defaultwidth,
	height = defaultheight,
	fullscreen = defaultfullscreen,
	frontend = defaultfrontend,
	port = 45678,
	grabmouse = true,
	showcursor = defaultshowcursor,
	debug = false,
	showfps = false,
	joystick = defaultjoystick,
	sound = true,
	soundengine = defaultsoundengine,
	persister = defaultpersister,
	network = defaultnetwork,
	texturesize = defaulttexturesize,
	referencetimefactor = defaultreferencetimefactor,
	damagethreshold = defaultdamagethreshold,
	fruitcollectdelayforanewlife = 15000,
	amountoffruitsforanewlife = 4,
	fruithitpoints = 10,
	waterparticle = false,
	npcflyingspeed = defaultnpcflyingspeed,
	particles = defaultparticles,
}

controllerbindings = {
	["ui"] = {
		--A = "drop",
		--B = "",
		X = "ui_execute",
		Y = "ui_focus_next",
		BACK = "ui_pop",
		--GUIDE = "",
		--START = "",
		--LEFTSTICK = "",
		--RIGHTSTICK = "",
		--LEFTSHOULDER = "",
		--RIGHTSHOULDER = "",
		--DPUP = "",
		--DPDOWN = "",
		--DPLEFT = "",
		--DPRIGHT = "",
	},
	["map"] = {
		A = "drop",
		--B = "",
		--X = "ui_execute",
		--Y = "ui_focus_next",
		--BACK = "ui_pop",
		--GUIDE = "",
		--START = "",
		--LEFTSTICK = "",
		--RIGHTSTICK = "",
		--LEFTSHOULDER = "",
		--RIGHTSHOULDER = "",
		--DPUP = "",
		--DPDOWN = "",
		--DPLEFT = "",
		--DPRIGHT = "",
	},
}

joystickbindings = {
	["ui"] = {},
	["map"] = {},
}

keybindings = {
	["ui"] = {
		LEFT = "ui_focus_prev",
		RIGHT = "ui_focus_next",
		UP = "ui_focus_prev",
		DOWN = "ui_focus_next",
		SPACE = "ui_execute",
		RETURN = "ui_execute",
		ESCAPE = "ui_pop",
		TAB = "ui_focus_next",
	},
	["map"] = {
		LEFT = "+move_left",
		RIGHT = "+move_right",
		UP = "+move_up",
		DOWN = "+move_down",
		SPACE = "drop",
		RETURN = "drop",
		ESCAPE = "ui_pop",
		TAB = "ui_focus_next",
	},
}

if isDebug() then
	keybindings["ui"]["."] = "screenshot"
	keybindings["map"]["."] = "screenshot"
	keybindings["map"]["BACKSPACE"] = "map_debug"
	keybindings["map"]["E"] = "map_open_in_editor"
	keybindings["map"]["X"] = "kill"
	keybindings["map"]["F"] = "finish"
end
