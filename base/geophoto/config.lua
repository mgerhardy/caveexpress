function isMobile()
	return isAndroid() or isIOS();
end

maxwidth = 1024
maxheight = 768

defaultwidth = -1
defaultheight = -1
defaultfullscreen = false
defaultpersister = "sqlite"
defaultfrontend = "sdl"
defaultsoundengine = "sdl"
defaultnetwork = true
defaultjoystick = false

if isHTML5() then
	defaultpersister = "nop"
	defaultfrontend = "opengl"
	defaultnetwork = false
end

if not isMobile() then
	-- workaround for now - remove me later
	defaultfrontend = "opengl"
	defaultjoystick = true
end

function getWidth()
	if isMobile() then
		return defaultwidth
	else
		if (defaultwidth == -1) then
			return maxwidth
		end
		return math.min(maxwidth, defaultwidth)
	end
end

function getHeight()
	if isMobile() then
		return defaultheight
	else
		if (defaultheight == -1) then
			return maxheight
		end
		return math.min(maxheight, defaultheight)
	end
end

settings = {
	width = getWidth(),
	height = getHeight(),
	fullscreen = defaultfullscreen,
	frontend = defaultfrontend,
	port = 45678,
	grabmouse = true,
	debug = false,
	showfps = not isMobile(),
	joystick = defaultjoystick,
	sound = true,
	soundengine = defaultsoundengine,
	persister = defaultpersister,
	network = defaultnetwork,
	difficulty = "normal",
	url = "http://notthisway.com/geophoto/server/app/api/round/$difficulty$/"
}

if isMobile() or isOUYA() then
	keybindings = {
		["ui"] = {
			AC_BACK = "ui_pop",
		},
		["map"] = {
			--MENU = "ui_push settings",
			AC_BACK = "ui_pop",
		},
	}

	if isOUYA() then
		keybindings["ui"]["PAUSE"] = "ui_pop"
		keybindings["map"]["PAUSE"] = "ui_pop"
	end
else
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
			PAGEDOWN = "zoom -0.1",
			PAGEUP = "zoom 0.1",
		},
	}

	if isDebug() then
		keybindings["ui"]["."] = "screenshot"
		keybindings["map"]["."] = "screenshot"
	end
end
