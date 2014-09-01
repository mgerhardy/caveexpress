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

if isMobile() then
	keybindings = {
		MENU = "ui_push settings",
	}
else
	keybindings = {
		LEFT = "+move_left",
		RIGHT = "+move_right",
		UP = "+move_up",
		DOWN = "+move_down",
		SPACE = "drop",
		ESCAPE = "ui_pop",
		MENU = "ui_push settings",
		TAB = "ui_focus_next",
		RETURN = "ui_execute;ui_focus_next",
	}
end

