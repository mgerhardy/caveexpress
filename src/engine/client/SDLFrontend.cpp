#include "SDLFrontend.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ClientConsole.h"
#include "engine/client/shaders/ShaderManager.h"
#include "engine/common/Version.h"
#include "engine/common/String.h"
#include "engine/common/EventHandler.h"
#include "engine/common/CommandSystem.h"
#include "engine/client/sound/Sound.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/FileSystem.h"
#include "engine/common/Pointers.h"
#include "engine/common/DateUtil.h"
#include "engine/common/Singleton.h"
#include "engine/GameRegistry.h"
#include "engine/common/Commands.h"
#include <SDL_image.h>
#include <SDL_platform.h>
#include <limits.h>

SDLFrontend::SDLFrontend (SharedPtr<IConsole> console) :
		IFrontend(), _eventHandler(nullptr), _numFrames(0), _time(0), _timeBase(0), _console(console), _softwareRenderer(false)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_window = nullptr;
	_haptic = nullptr;
#endif
#ifndef DISABLE_SDL_RENDERER
	_renderer = nullptr;
#endif

	_debugSleep = Config.getConfigVar("debugSleep", "0", true);
	Vector4Set(colorBlack, _color);
}

SDLFrontend::~SDLFrontend ()
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_haptic)
		SDL_HapticClose(_haptic);

//	for (int i = 0; i < _numJoysticks; ++i)
//		SDL_JoystickClose(_joysticks[i]);
#endif
#ifndef DISABLE_SDL_RENDERER
	if (_renderer)
		SDL_DestroyRenderer(_renderer);
#endif
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_DestroyWindow(_window);
	IMG_Quit();
#endif
}

void SDLFrontend::onPrepareBackground ()
{
	SoundControl.pause();
}

void SDLFrontend::onForeground ()
{
	SoundControl.resume();
}

void SDLFrontend::onJoystickDeviceRemoved (int32_t device)
{
	info(LOG_CLIENT, "joystick removed");
	initJoystickAndHaptic();
}

void SDLFrontend::onJoystickDeviceAdded (int32_t device)
{
	info(LOG_CLIENT, "joystick added");
	initJoystickAndHaptic();
}

void SDLFrontend::onWindowResize ()
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	int w, h;
	SDL_GetWindowSize(_window, &w, &h);
	_width = w;
	_height = h;

	updateViewport(0, 0, getWidth(), getHeight());
#endif
}

void SDLFrontend::setWindowTitle (const std::string& title)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetWindowTitle(_window, title.c_str());
#endif
}

void SDLFrontend::setVSync (bool vsync)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GL_SetSwapInterval(ConfigManager::get().isVSync() ? 1 : 0);
#endif
}

void SDLFrontend::update (uint32_t deltaTime)
{
#ifdef DEBUG
	const int sleepTime = abs(_debugSleep->getIntValue()) % 1000;
	if (0 < sleepTime) {
		SDL_Delay(sleepTime);
	}
#endif
	++_numFrames;
	_time += deltaTime;

	if (((_time - _timeBase) > 500 || _numFrames == 0) && Config.showFPS()) {
		setVSync(ConfigManager::get().isVSync());
		const double fps = _numFrames * 1000.0f / (_time - _timeBase);
		setWindowTitle(APPFULLNAME " (" + string::toString((int) fps) + ")");
		_timeBase = _time;
		_numFrames = 0;
	}

	_console->update(deltaTime);
	UI::get().update(deltaTime);
	SoundControl.update(deltaTime);
	ShaderManager::get().update(deltaTime);
}

bool SDLFrontend::setFrameCallback (int interval, void (*callback) (void*), void *callbackParam)
{
#if 0
#ifdef __IPHONEOS__
	// Set up the game to run in the window animation callback on iOS
	// so that Game Center and so forth works correctly.
	SDL_iPhoneSetAnimationCallback(_window, interval, callback, callbackParam);
	return true;
#endif
#endif

	return false;
}

void SDLFrontend::onMapLoaded ()
{
}

void SDLFrontend::onStart ()
{
	info(LOG_CLIENT, "sdl frontend is starting");
	UI::get().initStack();
}

void SDLFrontend::onInit ()
{
	renderBegin();
	ScopedPtr<Texture> ptr(new Texture("loading", this));
	const int x = getWidth() / 2 - ptr->getWidth() / 2;
	const int y = getHeight() / 2 - ptr->getHeight() / 2;
	renderImage(ptr, x, y, ptr->getWidth(), ptr->getHeight(), 0);
	renderEnd();
}

void SDLFrontend::shutdown ()
{
	// in case of an error the eventhandler might not yet be set
	if (_eventHandler != nullptr) {
		_eventHandler->removeObserver(_console.get());
		_eventHandler->removeObserver(this);
		_eventHandler = nullptr;
	}
	UI::get().shutdown();
	SoundControl.close();
}

bool SDLFrontend::handlesInput () const
{
	return _console->isActive();
}

void SDLFrontend::connect ()
{
	Singleton<GameRegistry>::getInstance().getGame()->connect();
}

void SDLFrontend::getTrimmed (const Texture* texture, int& x, int& y, int& w, int& h) const
{
	const TextureDefinitionTrim& trim = texture->getTrim();
	if (trim.trimmedWidth != trim.untrimmedWidth) {
		const float wAspect = w / static_cast<float>(trim.untrimmedWidth);
		x += trim.trimmedOffsetX * wAspect;
		w -= (trim.untrimmedWidth - trim.trimmedWidth) * wAspect;
	}
	if (trim.trimmedHeight != trim.untrimmedHeight) {
		const float hAspect = h / static_cast<float>(trim.untrimmedHeight);
		y += trim.trimmedOffsetY * hAspect;
		h -= (trim.untrimmedHeight - trim.trimmedHeight) * hAspect;
	}
}

void SDLFrontend::renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	if (!texture->isValid())
		return;

	getTrimmed(texture, x, y, w, h);

	const SDL_Rect destRect = { x, y, w, h };
	const TextureRect& r = texture->getSourceRect();
	const SDL_Rect srcRect = { r.x, r.y, r.w, r.h };
	SDL_Texture *t = static_cast<SDL_Texture*>(texture->getData());
	SDL_SetTextureAlphaMod(t, alpha * 255);
	SDL_SetTextureColorMod(t, _color[0] * 255, _color[1] * 255, _color[2] * 255);
	if (_softwareRenderer) {
		// angle is 0 here - because on the fly rotating is really expensive
		// TODO: create a lockup map here?
		if (SDL_RenderCopyEx(_renderer, t, &srcRect, &destRect, 0.0, nullptr,
				texture->isMirror() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE) != 0) {
			error(LOG_CLIENT, "could not render texture " + texture->getName());
			texture->setData(nullptr);
		}
	} else {
		if (SDL_RenderCopyEx(_renderer, t, &srcRect, &destRect, static_cast<double>(angle), nullptr,
				texture->isMirror() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE) != 0) {
			error(LOG_CLIENT, "could not render texture " + texture->getName());
			texture->setData(nullptr);
		}
	}
#endif
}

bool SDLFrontend::loadTexture (Texture *texture, const std::string& filename)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	FilePtr file = FS.getPic(filename + ".png");
	SDL_RWops *src = FS.createRWops(file->getURI());
	if (src == nullptr) {
		error(LOG_CLIENT, "could not load the file: " + file->getName());
		return false;
	}
	SDL_Surface *surface = IMG_Load_RW(src, 1);
	if (surface) {
		SDL_Texture *sdltexture = SDL_CreateTextureFromSurface(_renderer, surface);
		texture->setData(sdltexture);
		texture->setRect(0, 0, surface->w, surface->h);
		SDL_FreeSurface(surface);
		return texture->isValid();
	}

	sdlCheckError();
#endif
	return false;
}

void SDLFrontend::resetColor ()
{
	setColor(colorWhite);
}

void SDLFrontend::setColor (const Color& rgba)
{
	Vector4Set(rgba, _color);
}

void SDLFrontend::setSDLColor (const Color& rgba)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	const Uint8 r = rgba[0] * 255.0f;
	const Uint8 g = rgba[1] * 255.0f;
	const Uint8 b = rgba[2] * 255.0f;
	const Uint8 a = rgba[3] * 255.0f;
	if (SDL_SetRenderDrawColor(_renderer, r, g, b, a) == -1)
		sdlCheckError();
#endif
}

void SDLFrontend::bindTexture (Texture* texture, int textureUnit)
{
#ifndef DISABLE_SDL_RENDERER
	if (textureUnit != 0)
		error(LOG_CLIENT, "only one texture unit is supported in the sdl frontend");
	SDL_Texture *sdltexture = static_cast<SDL_Texture *>(texture->getData());
	SDL_GL_BindTexture(sdltexture, nullptr, nullptr);
#endif
}

void SDLFrontend::renderRect (int x, int y, int w, int h, const Color& color)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	if (w <= 0)
		w = getWidth();
	if (h <= 0)
		h = getHeight();
	const SDL_Rect r = { x, y, w, h };
	setSDLColor(color);
	if (SDL_RenderDrawRect(_renderer, &r) == -1)
		sdlCheckError();
#endif
}

void SDLFrontend::renderFilledRect (int x, int y, int w, int h, const Color& fillColor)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	if (w <= 0)
		w = getWidth();
	if (h <= 0)
		h = getHeight();
	const SDL_Rect r = { x, y, w, h };
	setSDLColor(fillColor);
	if (SDL_RenderFillRect(_renderer, &r) == -1)
		sdlCheckError();
#endif
}

void SDLFrontend::renderLine (int x1, int y1, int x2, int y2, const Color& color)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	setSDLColor(color);
	if (SDL_RenderDrawLine(_renderer, x1, y1, x2, y2) == -1)
		sdlCheckError();
#endif
}

void SDLFrontend::renderLineWithTexture (int x1, int y1, int x2, int y2, Texture* texture)
{
	const double angleInDegrees = getAngleBetweenPoints(x1, y1, x2, y2);
	renderImage(texture, x1, x2, texture->getWidth(), texture->getHeight(), angleInDegrees);
}

void SDLFrontend::updateViewport (int x, int y, int width, int height)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	SDL_RenderSetLogicalSize(_renderer, getWidth(), getHeight());

	ShaderManager::get().updateProjectionMatrix(width, height);
#endif
}

void SDLFrontend::enableScissor (int x, int y, int width, int height)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);
	const SDL_Rect rect = {x, y, width, height};
	SDL_RenderSetClipRect(_renderer, &rect);
#endif
}

void SDLFrontend::disableScissor ()
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);
	SDL_RenderSetClipRect(_renderer, nullptr);
#endif
}

void SDLFrontend::minimize ()
{
#ifndef DISABLE_SDL_RENDERER
	SDL_MinimizeWindow(_window);
#endif
}

void SDLFrontend::destroyTexture (void *data)
{
#ifndef DISABLE_SDL_RENDERER
	SDL_Texture *t = static_cast<SDL_Texture*>(data);
	SDL_DestroyTexture(t);
#endif
}

uint32_t SDLFrontend::getDisplayFormat () const
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(_renderer, &info);
	for (int i = 0; i < info.num_texture_formats; ++i) {
		if (!SDL_ISPIXELFORMAT_FOURCC(info.texture_formats[i]) && SDL_ISPIXELFORMAT_ALPHA(info.texture_formats[i])) {
			return info.texture_formats[i];
		}
	}

	const uint32_t format = info.texture_formats[0];
	return format;
#else
	return 0;
#endif
}

void SDLFrontend::renderBegin ()
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	resetColor();
	SDL_ClearError();
	setSDLColor(colorBlack);
	if (SDL_RenderClear(_renderer) == -1)
		sdlCheckError();
#endif
}

void SDLFrontend::renderEnd ()
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);
	SDL_RenderPresent(_renderer);
#endif
}

void SDLFrontend::render ()
{
	renderBegin();
	UI::get().render();
	_console->render();
	renderEnd();
}

void SDLFrontend::makeScreenshot (const std::string& filename)
{
#ifndef DISABLE_SDL_RENDERER
	assert(_renderer);

	SDL_Rect viewport;
	int bpp;
	Uint32 rmask, gmask, bmask, amask;

	SDL_RenderGetViewport(_renderer, &viewport);

	SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGBA8888, &bpp, &rmask, &gmask, &bmask, &amask);
	ScopedPtr<SDL_Surface> surface(SDL_CreateRGBSurface(0, viewport.w, viewport.h, bpp, rmask, gmask, bmask, amask));
	if (!surface)
		return;

	if (SDL_RenderReadPixels(_renderer, nullptr, surface->format->format, surface->pixels, surface->pitch) < 0)
		return;

	const std::string fullFilename = FS.getAbsoluteWritePath() + filename + "-" + dateutil::getDateString() + ".png";
	IMG_SavePNG(surface, fullFilename.c_str());
#endif
}

void SDLFrontend::setCursorPosition (int x, int y)
{
	// special case to disable the mouse cursor
	if (x == -1 && y == -1) {
		UI::get().setCursorPosition(x, y);
		return;
	}
	x = clamp(x, 0, _width);
	y = clamp(y, 0, _height);
	UI::get().setCursorPosition(x, y);
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (!SDL_GetRelativeMouseMode()) {
		SDL_WarpMouseInWindow(_window, x, y);
	}
#endif
}

void SDLFrontend::showCursor (bool show)
{
	UI::get().showCursor(show);
}

bool SDLFrontend::isFullscreen ()
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return SDL_GetWindowFlags(_window) & SDL_WINDOW_FULLSCREEN;
#else
	return true;
#endif
}

void SDLFrontend::setFullscreen (bool fullscreen)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	const SDL_bool b = fullscreen ? SDL_TRUE : SDL_FALSE;
	SDL_SetWindowFullscreen(_window, b);
#endif
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
static void logDebug (void *userdata, int category, SDL_LogPriority priority, const char *message)
{
	// SDLFrontend *frontend = static_cast<SDLFrontend*>(userdata);
	debug(LOG_CLIENT, String::format("SDL: %s (level: %i, category: %i)", message, priority, category));
}
#endif

#define INIT_Subsystem(flags, fatal) if (!SDL_WasInit(flags)) { if (SDL_Init(flags) == -1) { sdlCheckError(); if (fatal) return -1; } }

void SDLFrontend::initUI (ServiceProvider& serviceProvider)
{
	info(LOG_CLIENT, "init the ui");
	if (_eventHandler == nullptr)
		System.exit("No event handler given", 1);
	UI::get().init(serviceProvider, *_eventHandler, *this);

	info(LOG_CLIENT, "init the console");
	_console->init(this);
}

void SDLFrontend::initJoystickAndHaptic ()
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (_haptic != nullptr) {
		SDL_HapticClose(_haptic);
		_haptic = nullptr;
	}

	const int joysticks = SDL_NumJoysticks();
	SDL_Haptic *haptic = nullptr;
	for (int i = 0; i < joysticks; i++) {
		const char *name;
		if (SDL_IsGameController(i)) {
			name = SDL_GameControllerNameForIndex(i);
		} else {
			name = SDL_JoystickNameForIndex(i);
		}
		SDL_Joystick *joystick = SDL_JoystickOpen(i);
		info(LOG_CLIENT, String::format("found joystick %s", name ? name : "Unknown Joystick"));
		info(LOG_CLIENT, String::format("joystick axes: %i", SDL_JoystickNumAxes(joystick)));
		info(LOG_CLIENT, String::format("joystick hats: %i", SDL_JoystickNumHats(joystick)));
		info(LOG_CLIENT, String::format("joystick balls: %i", SDL_JoystickNumBalls(joystick)));
		info(LOG_CLIENT, String::format("joystick buttons: %i", SDL_JoystickNumButtons(joystick)));
		if (haptic == nullptr)
			haptic = SDL_HapticOpenFromJoystick(joystick);
	}
	if (!joysticks) {
		info(LOG_CLIENT, "no joysticks found");
	}

	info(LOG_CLIENT, String::format("found %i touch device(s)", SDL_GetNumTouchDevices()));

	info(LOG_CLIENT, String::format("%i haptic devices", SDL_NumHaptics()));
	if (haptic == nullptr && SDL_MouseIsHaptic()) {
		haptic = SDL_HapticOpenFromMouse();
	}
	if (haptic != nullptr) {
		const bool rumbleSupported = SDL_HapticRumbleSupported(haptic) && SDL_HapticRumbleInit(haptic) == 0;
		if (rumbleSupported) {
			info(LOG_CLIENT, "rumble support");
			_haptic = haptic;
		}
	}
	if (_haptic == nullptr) {
		info(LOG_CLIENT, "no rumble support");
	}
#endif
}

int SDLFrontend::init (int width, int height, bool fullscreen, EventHandler &eventHandler)
{
	if (width == -1 && height == -1)
		fullscreen = true;

	info(LOG_CLIENT,
			String::format("initializing: %i:%i - fullscreen: %s", width, height, fullscreen ? "true" : "false"));

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (Config.isDebug()) {
		SDL_LogSetOutputFunction(logDebug, this);
	}
#endif

	INIT_Subsystem(SDL_INIT_VIDEO, true);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	INIT_Subsystem(SDL_INIT_JOYSTICK, false);
	INIT_Subsystem(SDL_INIT_GAMECONTROLLER, false);
	INIT_Subsystem(SDL_INIT_HAPTIC, false);

	initJoystickAndHaptic();

	SDL_DisplayMode displayMode;
	SDL_GetDesktopDisplayMode(0, &displayMode);
	const char *name = SDL_GetPixelFormatName(displayMode.format);
	info(LOG_CLIENT, String::format("current desktop mode: %dx%d@%dHz (%s)",
			displayMode.w, displayMode.h, displayMode.refresh_rate, name));
	if (width == -1)
		width = displayMode.w;
	if (height == -1)
		height = displayMode.h;
#else
	if (width == -1)
		width = 1024;
	if (height == -1)
		height = 768;
	SDL_SetVideoMode(width, height, 16, SDL_OPENGL);
#endif

	setGLAttributes();
	setHints();

#if SDL_VERSION_ATLEAST(2, 0, 0)
	int doubleBuffered = 0;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doubleBuffered);

	info(LOG_CLIENT, String::format("doublebuffer: %s", doubleBuffered ? "activated" : "disabled"));

	int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
#ifdef __IPHONEOS__
	flags |= SDL_WINDOW_RESIZABLE;
#endif


#if defined __IPHONEOS__ || defined __ANDROID__
	if (fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
#else
	if (fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS;
#endif

	const int videoDrivers = SDL_GetNumVideoDrivers();
	for (int i = 0; i < videoDrivers; ++i) {
		info(LOG_CLIENT, String::format("available driver: %s", SDL_GetVideoDriver(i)));
	}

	info(LOG_CLIENT, String::format("driver: %s", SDL_GetCurrentVideoDriver()));
	const int displays = SDL_GetNumVideoDisplays();
	info(LOG_CLIENT, String::format("found %i display(s)", displays));
	if (fullscreen && displays > 1) {
		width = displayMode.w;
		height = displayMode.h;
		info(LOG_CLIENT, String::format("use fake fullscreen for the first display: %i:%i", width, height));
	}

	_window = SDL_CreateWindow(APPFULLNAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
	if (!_window) {
		sdlCheckError();
		return -1;
	}
#endif

	initRenderer();
	resetColor();
	GLContext::get().init();

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (SDL_SetWindowBrightness(_window, 1.0f) == -1)
		sdlCheckError();

	if (Config.isGrabMouse() && (!fullscreen || displays > 1)) {
		SDL_SetWindowGrab(_window, SDL_TRUE);
	}

	int screen = 0;
	int modes = SDL_GetNumDisplayModes(screen);
	info(LOG_CLIENT, "possible display modes:");
	for (int i = 0; i < modes; i++) {
		SDL_GetDisplayMode(screen, i, &displayMode);
		name = SDL_GetPixelFormatName(displayMode.format);
		info(LOG_CLIENT, String::format("%dx%d@%dHz %s",
				displayMode.w, displayMode.h, displayMode.refresh_rate, name));
	}

	// some platforms may override or hardcode the resolution - so
	// we have to query it here to get the actual resolution
	SDL_GetWindowSize(_window, &width, &height);
	if (SDL_SetRelativeMouseMode(SDL_TRUE) == -1)
		error(LOG_CLIENT, "no relative mouse mode support");
#else
	if (Config.isGrabMouse())
		SDL_WM_GrabInput(SDL_GRAB_ON);
#endif
	SDL_ShowCursor(0);
	info(LOG_CLIENT, String::format("actual resolution: %dx%d", width, height));
	setVSync(ConfigManager::get().isVSync());

	const int initState = IMG_Init(IMG_INIT_PNG);
	if (!(initState & IMG_INIT_PNG)) {
		sdlCheckError();
		return -1;
	}

	_width = width;
	_height = height;
	updateViewport(0, 0, getWidth(), getHeight());

	onInit();

	_eventHandler = &eventHandler;
	_eventHandler->registerObserver(_console.get());
	_eventHandler->registerObserver(this);

	info(LOG_CLIENT, "init the shader manager");
	ShaderManager::get().init();

	if (!Config.isSoundEnabled()) {
		info(LOG_CLIENT, "sound disabled");
	} else if (!SoundControl.init(true)) {
		error(LOG_CLIENT, "sound initialization failed");
	}

	return 0;
}

void SDLFrontend::initRenderer ()
{
#ifndef DISABLE_SDL_RENDERER
	info(LOG_CLIENT, "init sdl renderer");
	const int renderers = SDL_GetNumRenderDrivers();
	SDL_RendererInfo ri;
	for (int i = 0; i < renderers; i++) {
		SDL_GetRenderDriverInfo(i, &ri);
		info(LOG_CLIENT, String::format("available renderer %s", ri.name));
	}

#if defined(SDL_VIDEO_OPENGL_ES2)
	const std::string rendererStr = "opengles2";
#elif defined(SDL_VIDEO_OPENGL_ES)
	const std::string rendererStr = "opengles";
#elif defined(SDL_VIDEO_OPENGL)
	const std::string rendererStr = "opengl";
#else
#error "No supported renderer found"
#endif

	const ConfigVarPtr& renderer = Config.getConfigVar("renderer", rendererStr, true);
	const std::string& rendererValue = renderer->getValue();
	info(LOG_CLIENT, "try renderer: " + rendererValue);
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, rendererValue.c_str());
	_renderer = SDL_CreateRenderer(_window, -1, 0);
	SDL_GetRendererInfo(_renderer, &ri);

	SDL_RenderSetLogicalSize(_renderer, getWidth(), getHeight());

	_softwareRenderer = (ri.flags & SDL_RENDERER_SOFTWARE);

	info(LOG_CLIENT, String::format("actual renderer %s", ri.name));
	if (strcmp(ri.name, "opengles2")) {
		// disable shaders as they are currently built for glesv2
		ConfigManager::get().getConfigVar("shader")->setValue("false");
		info(LOG_CLIENT, "disable shaders for the current renderer");
	}
	info(LOG_CLIENT, String::format("max texture resolution: %i:%i", ri.max_texture_width, ri.max_texture_height));
	SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
#endif
}

void SDLFrontend::setGLAttributes ()
{
#ifndef DISABLE_SDL_RENDERER
	SDL_ClearError();
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
	sdlCheckError();
#ifdef __ANDROID__
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 6);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 6);
	sdlCheckError();
#else
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	sdlCheckError();
#endif
#ifdef __IPHONEOS__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	sdlCheckError();
#endif
#endif
}

void SDLFrontend::setHints ()
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
#ifdef __IPHONEOS__
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#endif
}

bool SDLFrontend::rumble (float strength, int lengthMillis)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (!_haptic) {
		return false;
	}

	if (SDL_HapticRumblePlay(_haptic, strength, lengthMillis) != 0) {
		sdlCheckError();
		return false;
	}
	return true;
#else
	return false;
#endif
}

bool SDLFrontend::isConsoleActive () const
{
	return _console->isActive();
}

#ifndef DISABLE_SDL_RENDERER
static int sortRenderFilledPolygon (const void* a, const void* b)
{
	return (*(const int*) a) - (*(const int*) b);
}
#endif

int SDLFrontend::renderFilledPolygon (int *vx, int *vy, int n, const Color& color)
{
	if (!vx || !vy || n < 3)
		return -1;

#ifndef DISABLE_SDL_RENDERER
	ScopedArrayPtr<int> ints(new int[n]);
	int miny = vy[0];
	int maxy = vy[0];
	for (int i = 1; i < n; i++) {
		if (vy[i] > maxy)
			maxy = vy[i];
		if (vy[i] < miny)
			miny = vy[i];
	}

	int result = 0;
	for (int y = miny; y <= maxy; y++) {
		int num_ints = 0;
		for (int i = 0; i < n; i++) {
			int ind1, ind2;
			if (i == 0) {
				ind1 = n - 1;
				ind2 = 0;
			} else {
				ind1 = i - 1;
				ind2 = i;
			}
			int x1;
			int x2;
			int y1 = vy[ind1];
			int y2 = vy[ind2];
			if (y1 < y2) {
				x1 = vx[ind1];
				x2 = vx[ind2];
			} else if (y1 > y2) {
				y2 = vy[ind1];
				y1 = vy[ind2];
				x2 = vx[ind1];
				x1 = vx[ind2];
			} else {
				continue;
			}

			if ((y >= y1 && y < y2) || (y == maxy && y <= y2 && y > y1)) {
				ints[num_ints++] = ((y - y1) * (x2 - x1) / (y2 - y1)) + x1;
			}
		}

		SDL_qsort(ints, num_ints, sizeof(int), sortRenderFilledPolygon);

		for (int i = 0; i < num_ints; i += 2) {
			const int start = ints[i];
			const int end = ints[i + 1];

			result |= SDL_RenderDrawLine(_renderer, start, y, end, y);
		}
	}
	return result;
#else
	return -1;
#endif
}

int SDLFrontend::renderPolygon (int *vx, int *vy, int n, const Color& color)
{
	if (n < 3 || vx == nullptr || vy == nullptr)
		return -1;

#ifndef DISABLE_SDL_RENDERER
	int result = 0;
	for (int i = 0; i < n; i++) {
		if (i == 0) {
			result |= SDL_RenderDrawLine(_renderer, vx[i], vy[i], vx[n - 1],
					vy[n - 1]);
		} else {
			result |= SDL_RenderDrawLine(_renderer, vx[i], vy[i], vx[i - 1],
					vy[i - 1]);
		}
	}

	return result;
#else
	return -1;
#endif
}
