#include "SDLFrontend.h"
#include "common/String.h"
#include "common/EventHandler.h"
#include "common/CommandSystem.h"
#include "sound/Sound.h"
#include "common/ConfigManager.h"
#include "common/FileSystem.h"
#include <memory>
#include "common/DateUtil.h"
#include "common/Singleton.h"
#include "game/GameRegistry.h"
#include "ui/BitmapFont.h"
#include "common/Commands.h"
#include "common/Application.h"
#include <SDL_image.h>
#include <SDL_platform.h>
#include <SDL_assert.h>
#include <limits.h>

struct TextureData {
	void* unused;
};

struct RenderTarget {
	int unused;
};

SDLFrontend::SDLFrontend (std::shared_ptr<IConsole> console) :
		IFrontend(), _eventHandler(nullptr), _numFrames(0), _time(0), _timeBase(0), _console(console), _softwareRenderer(false), _drawCalls(0)
{
	_window = nullptr;
	_haptic = nullptr;
	_renderer = nullptr;
	_renderToTexture = nullptr;

	_debugSleep = Config.getConfigVar("debugSleep", "0", true);
	Vector4Set(colorBlack, _color);
}

SDLFrontend::~SDLFrontend ()
{
	if (_haptic)
		SDL_HapticClose(_haptic);

//	for (int i = 0; i < _numJoysticks; ++i)
//		SDL_JoystickClose(_joysticks[i]);
	SDL_DestroyTexture(_renderToTexture);
	if (_renderer)
		SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);
	IMG_Quit();
}

void SDLFrontend::onPrepareBackground ()
{
//	SoundControl.pause();
}

void SDLFrontend::onForeground ()
{
//	SoundControl.resume();
}

void SDLFrontend::onJoystickDeviceRemoved (int32_t device)
{
	Log::info(LOG_GFX, "joystick removed");
	initJoystickAndHaptic();
}

void SDLFrontend::onJoystickDeviceAdded (int32_t device)
{
	Log::info(LOG_GFX, "joystick added");
	initJoystickAndHaptic();
}

void SDLFrontend::onWindowResize ()
{
	int w, h;
	SDL_GetWindowSize(_window, &w, &h);
	_width = w;
	_height = h;

	updateViewport(0, 0, getWidth(), getHeight());
}

void SDLFrontend::setWindowTitle (const std::string& title)
{
	SDL_SetWindowTitle(_window, title.c_str());
}

void SDLFrontend::setVSync (bool vsync)
{
	static bool lastState = !vsync;
	if (lastState == vsync)
		return;
	lastState = vsync;
	if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == -1)
		SDL_ClearError();
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

	_console->update(deltaTime);
	UI::get().update(deltaTime);
	SoundControl.update(deltaTime);

	const bool vsync = ConfigManager::get().isVSync();
	setVSync(vsync);
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
	Log::info(LOG_GFX, "sdl frontend is starting");
	UI::get().initStack();
}

void SDLFrontend::onInit ()
{
	renderBegin();
	std::unique_ptr<Texture> ptr(new Texture("loading", this));
	const int x = getWidth() / 2 - ptr->getWidth() / 2;
	const int y = getHeight() / 2 - ptr->getHeight() / 2;
	renderImage(ptr.get(), x, y, ptr->getWidth(), ptr->getHeight(), 0);
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
	const std::string command = CMD_CL_CONNECT " localhost " + string::toString(Config.getPort());
	Commands.executeCommandLine(command);
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
	SDL_assert(_renderer);

	if (texture == nullptr || !texture->isValid())
		return;

	getTrimmed(texture, x, y, w, h);

	const SDL_Rect destRect = { x, y, w, h };
	const TextureRect& r = texture->getSourceRect();
	const SDL_Rect srcRect = { r.x, r.y, r.w, r.h };
	SDL_Texture *t = reinterpret_cast<SDL_Texture*>(texture->getData());
	SDL_SetTextureAlphaMod(t, alpha * 255);
	SDL_SetTextureColorMod(t, _color[0] * 255, _color[1] * 255, _color[2] * 255);
	if (_softwareRenderer) {
		// angle is 0 here - because on the fly rotating is really expensive
		// TODO: create a lockup map here for one side of the rotation (180 degree) and do the other side
		// via horizontal flip to see if this is faster than rotating
		//angle = 0;
	}
	if (SDL_RenderCopyEx(_renderer, t, &srcRect, &destRect, static_cast<double>(angle), nullptr,
			texture->isMirror() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE) != 0) {
		Log::error(LOG_GFX, "could not render texture %s", texture->getName().c_str());
		texture->setData(nullptr);
	}
	++_drawCalls;
}

bool SDLFrontend::loadTexture (Texture *texture, const std::string& filename)
{
	SDL_assert(_renderer);

	const std::string file = FS.getFileFromURL("pics://" + filename + ".png")->getName();
	SDL_RWops *src = FS.createRWops(file);
	if (src == nullptr) {
		Log::error(LOG_GFX, "could not load the file: %s", file.c_str());
		return false;
	}
	SDL_Texture *sdltexture = IMG_LoadTexture_RW(_renderer, src, 1);
	if (sdltexture) {
		int w, h;
		SDL_QueryTexture(sdltexture, nullptr, nullptr, &w, &h);
		texture->setData(reinterpret_cast<TextureData*>(sdltexture));
		texture->setRect(0, 0, w, h);
		return texture->isValid();
	}

	sdlCheckError();
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
	SDL_assert(_renderer);

	const Uint8 r = rgba[0] * 255.0f;
	const Uint8 g = rgba[1] * 255.0f;
	const Uint8 b = rgba[2] * 255.0f;
	const Uint8 a = rgba[3] * 255.0f;
	if (SDL_SetRenderDrawColor(_renderer, r, g, b, a) == -1)
		sdlCheckError();
}

void SDLFrontend::bindTexture (Texture* texture, int textureUnit)
{
	if (textureUnit != 0)
		Log::error(LOG_GFX, "only one texture unit is supported in the sdl frontend");
	SDL_Texture *sdltexture = reinterpret_cast<SDL_Texture *>(texture->getData());
	SDL_GL_BindTexture(sdltexture, nullptr, nullptr);
}

void SDLFrontend::renderRect (int x, int y, int w, int h, const Color& color)
{
	SDL_assert(_renderer);

	if (w <= 0)
		w = getWidth();
	if (h <= 0)
		h = getHeight();
	const SDL_Rect r = { x, y, w, h };
	setSDLColor(color);
	if (SDL_RenderDrawRect(_renderer, &r) == -1)
		sdlCheckError();
	++_drawCalls;
}

void SDLFrontend::renderFilledRect (int x, int y, int w, int h, const Color& fillColor)
{
	SDL_assert(_renderer);

	if (w <= 0)
		w = getWidth();
	if (h <= 0)
		h = getHeight();
	const SDL_Rect r = { x, y, w, h };
	setSDLColor(fillColor);
	if (SDL_RenderFillRect(_renderer, &r) == -1)
		sdlCheckError();
	++_drawCalls;
}

void SDLFrontend::renderLine (int x1, int y1, int x2, int y2, const Color& color)
{
	SDL_assert(_renderer);

	setSDLColor(color);
	if (SDL_RenderDrawLine(_renderer, x1, y1, x2, y2) == -1)
		sdlCheckError();
	++_drawCalls;
}

void SDLFrontend::renderLineWithTexture (int x1, int y1, int x2, int y2, Texture* texture)
{
	const double angleInDegrees = getAngleBetweenPoints(x1, y1, x2, y2);
	renderImage(texture, x1, x2, texture->getWidth(), texture->getHeight(), angleInDegrees);
}

void SDLFrontend::updateViewport (int x, int y, int width, int height)
{
	SDL_assert(_renderer);

	SDL_RenderSetLogicalSize(_renderer, getWidth(), getHeight());
	SDL_DestroyTexture(_renderToTexture);
	_renderToTexture = SDL_CreateTexture(_renderer, getDisplayFormat(), SDL_TEXTUREACCESS_TARGET, getWidth(), getHeight());
}

void SDLFrontend::enableScissor (int x, int y, int width, int height)
{
	SDL_assert(_renderer);
	const SDL_Rect rect = {x, y, width, height};
	SDL_RenderSetClipRect(_renderer, &rect);
}

void SDLFrontend::disableScissor ()
{
	SDL_assert(_renderer);
	SDL_RenderSetClipRect(_renderer, nullptr);
}

void SDLFrontend::minimize ()
{
	SDL_MinimizeWindow(_window);
}

void SDLFrontend::destroyTexture (TextureData *data)
{
	SDL_Texture *t = reinterpret_cast<SDL_Texture*>(data);
	SDL_DestroyTexture(t);
}

uint32_t SDLFrontend::getDisplayFormat () const
{
	SDL_assert(_renderer);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(_renderer, &info);
	for (unsigned int i = 0; i < info.num_texture_formats; ++i) {
		if (!SDL_ISPIXELFORMAT_FOURCC(info.texture_formats[i]) && SDL_ISPIXELFORMAT_ALPHA(info.texture_formats[i])) {
			return info.texture_formats[i];
		}
	}

	const uint32_t format = info.texture_formats[0];
	return format;
}

void SDLFrontend::renderBegin ()
{
	_drawCalls = 0;
	SDL_assert(_renderer);

	resetColor();
	SDL_ClearError();
	setSDLColor(colorBlack);
	if (SDL_RenderClear(_renderer) == -1)
		sdlCheckError();
}

void SDLFrontend::renderEnd ()
{
	SDL_assert(_renderer);
	SDL_RenderPresent(_renderer);
}

RenderTarget* SDLFrontend::renderToTexture (int x, int y, int w, int h)
{
	static RenderTarget target;
	SDL_SetRenderTarget(_renderer, _renderToTexture);
	SDL_RenderClear(_renderer);
	return &target;
}

bool SDLFrontend::disableRenderTarget (RenderTarget* target)
{
	SDL_SetRenderTarget(_renderer, nullptr);
	return true;
}

bool SDLFrontend::renderTarget (RenderTarget* target)
{
	disableRenderTarget(target);
	SDL_RenderCopy(_renderer, _renderToTexture, nullptr, nullptr);
	++_drawCalls;
	return true;
}

void SDLFrontend::render ()
{
	renderBegin();
	UI::get().render();
	_console->render();

	static std::string fpsStr;
	const bool showFps = Config.showFPS();
	static int lastDrawCalls = _drawCalls;
	const uint32_t lastFpsTime = _time - _timeBase;
	if (lastFpsTime > 1000 && showFps) {
		const bool vsync = ConfigManager::get().isVSync();
		const double fps = _numFrames * 1000.0f / lastFpsTime;
		static ConfigVar* fpsLimit = Config.getConfigVar("fpslimit").get();
		static ConfigVar* frontend = Config.getConfigVar("frontend").get();
		fpsStr = string::format("%.2f (vsync: %s, %s %i, frontend: %s, drawcalls: %i)", fps, vsync ? "true" : "false",
				fpsLimit->getName().c_str(), fpsLimit->getIntValue(), frontend->getValue().c_str(), lastDrawCalls);
		_timeBase = _time - (lastFpsTime - 1000);
		_numFrames = 0;
	}
	if (showFps) {
		UI::get().getFont()->print(fpsStr, colorWhite, 0, 0);
	}

	renderEnd();
	lastDrawCalls = _drawCalls;
}

void SDLFrontend::makeScreenshot (const std::string& filename)
{
	SDL_assert(_renderer);

	SDL_Rect viewport;
	int bpp;
	Uint32 rmask, gmask, bmask, amask;

	SDL_RenderGetViewport(_renderer, &viewport);

	SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGBA8888, &bpp, &rmask, &gmask, &bmask, &amask);
	std::unique_ptr<SDL_Surface> surface(SDL_CreateRGBSurface(0, viewport.w, viewport.h, bpp, rmask, gmask, bmask, amask));
	if (!surface)
		return;

	if (SDL_RenderReadPixels(_renderer, nullptr, surface->format->format, surface->pixels, surface->pitch) < 0)
		return;

	const std::string fullFilename = FS.getAbsoluteWritePath() + filename + "-" + dateutil::getDateString() + ".png";
	IMG_SavePNG(surface.get(), fullFilename.c_str());
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
	if (!SDL_GetRelativeMouseMode() && Config.isGrabMouse()) {
		SDL_WarpMouseInWindow(_window, x, y);
	}
}

void SDLFrontend::showCursor (bool show)
{
	UI::get().showCursor(show);
}

bool SDLFrontend::isFullscreen ()
{
	return SDL_GetWindowFlags(_window) & SDL_WINDOW_FULLSCREEN;
}

void SDLFrontend::setFullscreen (bool fullscreen)
{
	const SDL_bool b = fullscreen ? SDL_TRUE : SDL_FALSE;
	SDL_SetWindowFullscreen(_window, b);
}

#define INIT_Subsystem(flags, fatal) if (!SDL_WasInit(flags)) { if (SDL_Init(flags) == -1) { sdlCheckError(); if (fatal) return -1; } }

void SDLFrontend::initUI (ServiceProvider& serviceProvider)
{
	Log::info(LOG_GFX, "init the ui");
	if (_eventHandler == nullptr)
		System.exit("No event handler given", 1);
	UI::get().init(serviceProvider, *_eventHandler, *this);

	Log::info(LOG_GFX, "init the console");
	_console->init(this);
}

void SDLFrontend::initJoystickAndHaptic ()
{
	if (_haptic != nullptr) {
		SDL_HapticClose(_haptic);
		_haptic = nullptr;
	}

	const int joysticks = SDL_NumJoysticks();
	SDL_Haptic *haptic = nullptr;
	for (int i = 0; i < joysticks; i++) {
		const char *name;
		SDL_Joystick *joystick;
		if (SDL_IsGameController(i)) {
			name = SDL_GameControllerNameForIndex(i);
			SDL_GameController* controller = SDL_GameControllerOpen(i);
			joystick = SDL_GameControllerGetJoystick(controller);
		} else {
			name = SDL_JoystickNameForIndex(i);
			joystick = SDL_JoystickOpen(i);
		}
		Log::info(LOG_GFX, "found joystick %s", name ? name : "Unknown Joystick");
		Log::debug(LOG_GFX, "joystick axes: %i", SDL_JoystickNumAxes(joystick));
		Log::debug(LOG_GFX, "joystick hats: %i", SDL_JoystickNumHats(joystick));
		Log::debug(LOG_GFX, "joystick balls: %i", SDL_JoystickNumBalls(joystick));
		Log::debug(LOG_GFX, "joystick buttons: %i", SDL_JoystickNumButtons(joystick));
		if (haptic == nullptr)
			haptic = SDL_HapticOpenFromJoystick(joystick);
	}
	if (!joysticks) {
		Log::info(LOG_GFX, "no joysticks found");
	}

	Log::info(LOG_GFX, "%i haptic devices", SDL_NumHaptics());
	if (haptic == nullptr && SDL_MouseIsHaptic()) {
		haptic = SDL_HapticOpenFromMouse();
	}
	if (haptic != nullptr) {
		const bool rumbleSupported = SDL_HapticRumbleSupported(haptic) && SDL_HapticRumbleInit(haptic) == 0;
		if (rumbleSupported) {
			Log::info(LOG_GFX, "rumble support");
			_haptic = haptic;
		}
	}
	if (_haptic == nullptr) {
		Log::info(LOG_GFX, "no rumble support");
	}
}

int SDLFrontend::init (int width, int height, bool fullscreen, EventHandler &eventHandler)
{
	if (width == -1 && height == -1)
		fullscreen = true;

	Log::info(LOG_GFX, "initializing: %i:%i - fullscreen: %s", width, height, fullscreen ? "true" : "false");

	INIT_Subsystem(SDL_INIT_VIDEO, true);

	INIT_Subsystem(SDL_INIT_JOYSTICK, false);
	INIT_Subsystem(SDL_INIT_GAMECONTROLLER, false);
	INIT_Subsystem(SDL_INIT_HAPTIC, false);

	const FilePtr& file = FS.getFile("gamecontrollerdb.txt");
	SDL_RWops* controllerDb = FS.createRWops(file->getName());
	if (controllerDb) {
		SDL_GameControllerAddMappingsFromRW(controllerDb, 1);
	} else {
		Log::info(LOG_GFX, "Could not update gamecontroller database. gamecontrollerdb.txt not found.");
	}

	Log::info(LOG_GFX, "found %i touch device(s)", SDL_GetNumTouchDevices());

	SDL_DisplayMode displayMode;
	SDL_GetDesktopDisplayMode(0, &displayMode);
	const char *name = SDL_GetPixelFormatName(displayMode.format);
	Log::info(LOG_GFX, "current desktop mode: %dx%d@%dHz (%s)",
			displayMode.w, displayMode.h, displayMode.refresh_rate, name);
	if (width == -1)
		width = displayMode.w;
	if (height == -1)
		height = displayMode.h;

	setGLAttributes();
	setHints();

	int doubleBuffered = 0;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doubleBuffered);

	Log::info(LOG_GFX, "doublebuffer: %s", doubleBuffered ? "activated" : "disabled");

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
		Log::info(LOG_GFX, "available driver: %s", SDL_GetVideoDriver(i));
	}

	Log::info(LOG_GFX, "driver: %s", SDL_GetCurrentVideoDriver());
	const int displays = SDL_GetNumVideoDisplays();
	Log::info(LOG_GFX, "found %i display(s)", displays);
	if (fullscreen && displays > 1) {
		width = displayMode.w;
		height = displayMode.h;
		Log::info(LOG_GFX, "use fake fullscreen for the first display: %i:%i", width, height);
	}

	const char *title = Singleton<Application>::getInstance().getName().c_str();
	_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
	if (!_window) {
		sdlCheckError();
		return -1;
	}

	SDL_DisableScreenSaver();

	initRenderer();
	resetColor();

	if (SDL_SetWindowBrightness(_window, 1.0f) == -1)
		sdlCheckError();

	if (Config.isGrabMouse() && (!fullscreen || displays > 1)) {
		SDL_SetWindowGrab(_window, SDL_TRUE);
	}

	int screen = 0;
	int modes = SDL_GetNumDisplayModes(screen);
	Log::info(LOG_GFX, "possible display modes:");
	for (int i = 0; i < modes; i++) {
		SDL_GetDisplayMode(screen, i, &displayMode);
		name = SDL_GetPixelFormatName(displayMode.format);
		Log::info(LOG_GFX, "%dx%d@%dHz %s", displayMode.w, displayMode.h, displayMode.refresh_rate, name);
	}

	// some platforms may override or hardcode the resolution - so
	// we have to query it here to get the resolution
	SDL_GetWindowSize(_window, &width, &height);
	if (SDL_SetRelativeMouseMode(SDL_TRUE) == -1)
		Log::error(LOG_GFX, "no relative mouse mode support");

	SDL_ShowCursor(0);
	Log::info(LOG_GFX, "resolution: %dx%d", width, height);
	setVSync(ConfigManager::get().isVSync());

	const int initState = IMG_Init(IMG_INIT_PNG);
	if (!(initState & IMG_INIT_PNG)) {
		sdlCheckError();
		System.exit("No png support", 1);
	}

	_width = width;
	_height = height;
	updateViewport(0, 0, getWidth(), getHeight());

	onInit();

	_eventHandler = &eventHandler;
	_eventHandler->registerObserver(_console.get());
	_eventHandler->registerObserver(this);

	if (!Config.isSoundEnabled()) {
		Log::info(LOG_GFX, "sound disabled");
	} else if (!SoundControl.init(true)) {
		Log::error(LOG_GFX, "sound initialization failed");
	}

	return 0;
}

void SDLFrontend::toggleGrabMouse () {
	bool grabMouse;
#if SDL_VERSION_ATLEAST(2, 0, 4)
	grabMouse = SDL_GetGrabbedWindow() == _window;
#else
	grabMouse = Config.isGrabMouse();
#endif
	SDL_SetWindowGrab(_window, grabMouse ? SDL_FALSE : SDL_TRUE);
	if (grabMouse)
		Log::info(LOG_GFX, "Mouse grab is now deactivated");
	else
		Log::info(LOG_GFX, "Mouse grab is now activated");
	Config.setGrabMouse(!grabMouse);
}

void SDLFrontend::initRenderer ()
{
	Log::info(LOG_GFX, "init sdl renderer");
	const int renderers = SDL_GetNumRenderDrivers();
	SDL_RendererInfo ri;
	for (int i = 0; i < renderers; i++) {
		SDL_GetRenderDriverInfo(i, &ri);
		Log::info(LOG_GFX, "available renderer %s", ri.name);
	}

#if defined(SDL_VIDEO_RENDER_D3D)
	const std::string rendererStr = "direct3d";
#elif defined(SDL_VIDEO_RENDER_D3D11)
	const std::string rendererStr = "direct3d11";
#elif defined(SDL_VIDEO_RENDER_OGL)
	const std::string rendererStr = "opengl";
#elif defined(SDL_VIDEO_RENDER_OGL_ES2)
	const std::string rendererStr = "opengles2";
#elif defined(SDL_VIDEO_RENDER_OGL_ES)
	const std::string rendererStr = "opengles";
#else
	Log::info(LOG_GFX, "compiled without hardware accelerated renderers");
	const std::string rendererStr = "software";
#endif

	const ConfigVarPtr& renderer = Config.getConfigVar("renderer", rendererStr, true);
	const std::string& rendererValue = renderer->getValue();
	Log::info(LOG_GFX, "try sdl renderer: %s", rendererValue.c_str());
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, rendererValue.c_str());
	_renderer = SDL_CreateRenderer(_window, -1, 0);
	SDL_GetRendererInfo(_renderer, &ri);

	SDL_RenderSetLogicalSize(_renderer, getWidth(), getHeight());

	_softwareRenderer = (ri.flags & SDL_RENDERER_SOFTWARE);

	Log::info(LOG_GFX, "got renderer: %s", ri.name);
	Log::info(LOG_GFX, "max texture resolution: %i:%i", ri.max_texture_width, ri.max_texture_height);
	SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
}

void SDLFrontend::setGLAttributes ()
{
	if (_softwareRenderer)
		return;
	SDL_ClearError();
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
	sdlCheckError();
	const int r = Config.getConfigVar("red")->getIntValue();
	const int g = Config.getConfigVar("green")->getIntValue();
	const int b = Config.getConfigVar("blue")->getIntValue();
	Log::info(LOG_GFX, "r: %i, g: %i, b: %i", r, g, b);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, r);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, g);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, b);
	sdlCheckError();
#ifdef __IPHONEOS__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	sdlCheckError();
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	sdlCheckError();
#endif
}

void SDLFrontend::setHints ()
{
#ifdef __IPHONEOS__
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
}

bool SDLFrontend::rumble (float strength, int lengthMillis)
{
	if (!_haptic) {
		return false;
	}

	if (SDL_HapticRumblePlay(_haptic, strength, lengthMillis) != 0) {
		sdlCheckError();
		return false;
	}
	return true;
}

bool SDLFrontend::isConsoleActive () const
{
	return _console->isActive();
}

static int sortRenderFilledPolygon (const void* a, const void* b)
{
	return (*(const int*) a) - (*(const int*) b);
}

int SDLFrontend::renderFilledPolygon (int *vx, int *vy, int n, const Color& color)
{
	if (!vx || !vy || n < 3)
		return -1;

	if (n == 4) {
		int xEqual = 0;
		int yEqual = 0;
		int minx = 10000000;
		int miny = 10000000;
		int maxx = 0;
		int maxy = 0;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				if (i == j)
					continue;
				if (vx[i] == vx[j]) {
					++xEqual;
				}
				if (vy[i] == vy[j]) {
					++yEqual;
				}
				minx = std::min(minx, vx[i]);
				maxx = std::max(maxx, vx[i]);
				miny = std::min(miny, vy[i]);
				maxy = std::max(maxy, vy[i]);
			}
		}
		if (xEqual == 2 && yEqual == 2) {
			const int w = maxx - minx;
			const int h = maxy - miny;
			renderFilledRect(minx, miny, w, h, color);
			return 0;
		}
	}

	setSDLColor(color);

	std::unique_ptr<int[]> ints(new int[n]);
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

		SDL_qsort(ints.get(), num_ints, sizeof(int), sortRenderFilledPolygon);

		for (int i = 0; i < num_ints; i += 2) {
			const int start = ints[i];
			const int end = ints[i + 1];

			result |= SDL_RenderDrawLine(_renderer, start, y, end, y);
			++_drawCalls;
		}
	}
	return result;
}

int SDLFrontend::renderPolygon (int *vx, int *vy, int n, const Color& color)
{
	if (n < 3 || vx == nullptr || vy == nullptr)
		return -1;

	setSDLColor(color);

	int result = 0;
	for (int i = 0; i < n; i++) {
		if (i == 0) {
			result |= SDL_RenderDrawLine(_renderer, vx[i], vy[i], vx[n - 1],
					vy[n - 1]);
		} else {
			result |= SDL_RenderDrawLine(_renderer, vx[i], vy[i], vx[i - 1],
					vy[i - 1]);
		}
		++_drawCalls;
	}

	return result;
}
