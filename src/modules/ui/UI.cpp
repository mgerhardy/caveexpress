#include "UI.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/BitmapFont.h"
#include "ui/windows/UIPopupWindow.h"
#include "ui/FontDefinition.h"
#include "common/EventHandler.h"
#include "common/IFrontend.h"
#include "common/Log.h"
#include "common/CommandSystem.h"
#include "common/Commands.h"
#include "common/ConfigManager.h"
#include "service/ServiceProvider.h"
#include "common/Singleton.h"
#include "common/FileSystem.h"
#include "game/GameRegistry.h"
#include "common/gestures/ZoomIn.h"
#include "common/gestures/ZoomOut.h"
#include <SDL.h>
#include <SDL_stdinc.h>

static inline int coordinateScaleX(int x, float scale, IFrontend* frontend) {
	const float offset = (float)(x + frontend->getCoordinateOffsetX());
	const float offsetScaled = offset / frontend->getWidthScale();
	const int scaledCoord = offsetScaled * scale;
	return scaledCoord;
}

static inline int coordinateScaleY(int y, float scale, IFrontend* frontend) {
	const float offset = (float)(y + frontend->getCoordinateOffsetY());
	const float offsetScaled = offset / frontend->getHeightScale();
	const int scaledCoord = offsetScaled * scale;
	return scaledCoord;
}

UI::UI () :
		_serviceProvider(nullptr), _eventHandler(nullptr), _frontend(nullptr), _cursor(true), _showCursor(false), _cursorX(
				-1), _cursorY(-1), _motionFinger(false), _restart(false), _delayedPop(false), _noPushAllowed(false), _time(0),
				_lastJoystickMoveTime(0), _lastJoystickMovementValue(0), _rotateFonts(true), _shutdown(false)
{
	_threadId = SDL_ThreadID();
}

UI::~UI ()
{
	popMain();
}

UI& UI::get ()
{
	static UI ui;
	SDL_assert_always(ui._threadId == SDL_ThreadID());
	return ui;
}

void UI::initRestart ()
{
	_restart = true;
}

void UI::restart ()
{
	if (!_restart)
		return;
	_restart = false;

	ServiceProvider *serviceProvider = _serviceProvider;
	EventHandler *eventHandler = _eventHandler;
	IFrontend *frontend = _frontend;

	const std::string root = getRootWindow() != nullptr ? getRootWindow()->getId() : "";

	shutdown();
	init(*serviceProvider, *eventHandler, *frontend);

	if (root.empty())
		return;
	pushRoot(root);
}

void UI::shutdown ()
{
	_shutdown = true;
	System.track("step", "shutdownui");
	// we might have temp windows on the stack
	popMain();
	for (UIWindowMapIter i = _windows.begin(); i != _windows.end(); ++i) {
		delete i->second;
	}
	_windows.clear();
	// in case of an error the eventhandler might not yet be set
	if (_eventHandler != nullptr) {
		_eventHandler->removeObserver(this);
		_eventHandler = nullptr;
	}
	Singleton<TextureCache>::getInstance().shutdown();
	_spriteCache.shutdown();

	_stack.clear();
	_fonts.clear();

	Commands.removeCommand(CMD_UI_PRINTSTACK);
	Commands.removeCommand(CMD_UI_PUSH);
	Commands.removeCommand(CMD_UI_POP);
	Commands.removeCommand(CMD_UI_FOCUS_NEXT);
	Commands.removeCommand(CMD_UI_FOCUS_PREV);
	Commands.removeCommand(CMD_UI_EXECUTE);
	Commands.removeCommand(CMD_UI_RESTART);
	_shutdown = false;
}

BitmapFontPtr UI::getFont (const std::string& font) const
{
	if (font.empty())
		return getFont(DEFAULT_FONT);
	Fonts::const_iterator i = _fonts.find(font);
	if (i == _fonts.end())
		return BitmapFontPtr();
	return i->second;
}

bool UI::initLanguage (const std::string& language)
{
	_languageMap.clear();
	if (language.empty())
		return false;
	const FilePtr& filePtr = FS.getFileFromURL("languages://" + language + ".lang");
	char *buffer;
	int fileLen = filePtr->read((void **) &buffer);
	std::unique_ptr<char[]> p(buffer);
	if (!buffer || fileLen <= 0) {
		Log::error(LOG_UI, "could not load language %s", language.c_str());
		return false;
	}
	std::string str(buffer, fileLen);
	std::vector<std::string> lines;
	string::splitString(str, lines, "\n");
	for (const auto& line : lines) {
		std::vector<std::string> tuple;
		string::splitString(line, tuple, "|");
		if (tuple.size() != 2)
			continue;
		_languageMap[tuple[0]] = tuple[1];
	}
	Log::info(LOG_UI, "loaded language '%s' with %i entries", language.c_str(), (int)_languageMap.size());
	return true;
}

const std::string UI::translate (const std::string& in) const
{
	LanguageMap::const_iterator i = _languageMap.find(in);
	if (i == _languageMap.end()) {
		Log::error(LOG_UI, "Missing translation for: %s", in.c_str());
		return in;
	}

	return i->second;
}

void UI::init (ServiceProvider& serviceProvider, EventHandler &eventHandler, IFrontend &frontend)
{
	System.track("step", "initui");
	const std::string& language = Config.getLanguage();
	if (!initLanguage(language))
		initLanguage("en_GB");
	Commands.registerCommandVoid(CMD_UI_PRINTSTACK, bindFunctionVoid(UI::printStack));
	Commands.registerCommandString(CMD_UI_PUSH, bindFunction(UI::pushCmd));
	Commands.registerCommandVoid(CMD_UI_RESTART, bindFunctionVoid(UI::initRestart));
	Commands.registerCommandVoid(CMD_UI_POP, bindFunctionVoid(UI::pop));
	Commands.registerCommand(CMD_UI_FOCUS_NEXT, bindFunction(UI::focusNext));
	Commands.registerCommand(CMD_UI_FOCUS_PREV, bindFunction(UI::focusPrev));
	Commands.registerCommandVoid(CMD_UI_EXECUTE, bindFunctionVoid(UI::runFocusNode));
	_mouseSpeed = Config.getConfigVar("mousespeed", "0.2");
	_showCursor = Config.getConfigVar("showcursor", System.wantCursor() ? "true" : "false", true)->getBoolValue();
	_cursor = _showCursor;
	if (_cursor)
		Log::info(LOG_UI, "enable cursor");
	else
		Log::info(LOG_UI, "disable cursor");

	_serviceProvider = &serviceProvider;
	_eventHandler = &eventHandler;
	_frontend = &frontend;
	eventHandler.registerObserver(this);

	Log::info(LOG_UI, "init the texture cache with %s", serviceProvider.getTextureDefinition().getTextureSize().c_str());
	Singleton<TextureCache>::getInstance().init(_frontend, serviceProvider.getTextureDefinition());
	_spriteCache.init();

	FontDefinition& fontDef = Singleton<FontDefinition>::getInstance();
	FontDefMapConstIter i = fontDef.begin();
	for (; i != fontDef.end(); ++i) {
		_fonts[i->second->id] = BitmapFontPtr(new BitmapFont(i->second, _frontend));
	}

	Singleton<GameRegistry>::getInstance().getGame()->initUI(_frontend, serviceProvider);

	_mouseCursor = loadTexture("mouse");

	loadGesture(zoominGesture, SDL_arraysize(zoominGesture));
	loadGesture(zoomoutGesture, SDL_arraysize(zoomoutGesture));
}

bool UI::loadGesture (const unsigned char* data, int length)
{
	SDL_RWops* rwops = SDL_RWFromConstMem(static_cast<const void*>(data), length);
	const int n = SDL_LoadDollarTemplates(-1, rwops);
	SDL_RWclose(rwops);
	if (n == -1) {
		Log::error(LOG_UI, "Failed to load gesture: %s", SDL_GetError());
		return false;
	} else if (n == 0) {
		Log::info(LOG_UI, "Could not load gesture");
		return false;
	}

	Log::info(LOG_UI, "Loaded gesture");
	return true;
}

void UI::initStack ()
{
	if (!_stack.empty())
		return;

	// push the main window onto the stack
	push(UI_WINDOW_MAIN);
	if (!Config.isModeSelected())
		push(UI_WINDOW_MODE_SELECTION);
}

void UI::addWindow (UIWindow *window)
{
	Log::info(LOG_UI, "Register window %s", window->getId().c_str());
	_windows[window->getId()] = window;
}

void UI::showCursor (bool show)
{
	// check if the system wants a cursor
	if (!_showCursor) {
		Log::debug(LOG_UI, "ignore show cursor call because the system does not want a cursor");
		return;
	}
	if (show)
		Log::debug(LOG_UI, "show the cursor");
	else
		Log::debug(LOG_UI, "hide the cursor");
	_cursor = show;
}

bool UI::isCursorVisible () const
{
	return _cursor;
}

void UI::progressInit (int steps, const std::string& text)
{
	_progress.steps = steps;
	_progress.text = text;
	_progress.step = 0;
	_progress.active = true;
}

void UI::progressStep (const std::string& text)
{
	++_progress.step;
	if (_frontend != nullptr)
		_frontend->render();
}

void UI::progressDone ()
{
	_progress.active = false;
}

void UI::renderProgress () const
{
	const int w = _frontend->getWidth() / 2;
	const int h = 30;
	const int x = _frontend->getWidth() / 2 - w / 2;
	const int y = _frontend->getHeight() / 2 - h / 2;
	_frontend->renderFilledRect(x, y, w, h, colorWhite);

	if (_progress.steps > 0 && _progress.step > 0) {
		const float factor = std::min(1.0f, static_cast<float>(_progress.step) / static_cast<float>(_progress.steps));
		const int width = w * factor;
		if (width > 0) {
			_frontend->renderFilledRect(x, y, width, h, colorGray);
		}
	}
}

void UI::render ()
{
	std::vector<UIWindow*> renderOrder;
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		renderOrder.push_back(window);
		// we can skip here
		if (window->isFullscreen())
			break;
	}

	for (std::vector<UIWindow*>::reverse_iterator i = renderOrder.rbegin(); i != renderOrder.rend(); ++i) {
		(*i)->render(0, 0);
	}

	// TODO: move cursor and progress bar into frontend
	if (_progress.active) {
		renderProgress();
	}

	if (_cursorX != -1 && _cursorY != -1 && _cursor) {
		const int w = _mouseCursor->getWidth();
		const int h = _mouseCursor->getHeight();
		_frontend->renderImage(_mouseCursor.get(), _cursorX, _cursorY, w, h, 0, 1.0f);
	}

	const bool debug = Config.getConfigVar("debugui")->getBoolValue();
	if (debug) {
		const BitmapFontPtr& font = getFont();
		const std::string s = string::format("%i:%i", _cursorX, _cursorY);
		font->print(s, colorWhite, 0, 0);
	}
}

void UI::update (uint32_t deltaTime)
{
	_time += deltaTime;
	if (_delayedPop) {
		_delayedPop = false;
		pop();
	}
	if (_restart)
		restart();

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		window->update(deltaTime);
	}

	if (!_rotateFonts)
		return;
	for (Fonts::iterator i = _fonts.begin(); i != _fonts.end(); ++i) {
		i->second->update(deltaTime);
	}
}

void UI::onWindowResize ()
{
	for (UIStackIter i = _stack.begin(); i != _stack.end(); ++i) {
		UIWindow* window = *i;
		window->onWindowResize();
	}
}

bool UI::onKeyRelease (int32_t key)
{
	if (_restart)
		return false;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onKeyRelease(key))
			return true;
		if (window->isModal() || window->isFullscreen())
			break;
	}

	return false;
}

bool UI::onKeyPress (int32_t key, int16_t modifier)
{
	if (_restart)
		return false;

	Log::debug(LOG_UI, "UI received key press event for key %i with modifier %i", key, modifier);
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onKeyPress(key, modifier))
			return true;
		if (window->isModal() || window->isFullscreen())
			break;
	}

	Log::debug(LOG_UI, "UI didn't handle key press event for key %i with modifier %i", key, modifier);
	return false;
}

bool UI::onTextInput (const std::string& text)
{
	if (_restart)
		return false;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onTextInput(text))
			return true;
		if (window->isModal() || window->isFullscreen())
			break;
	}
	return false;
}

bool UI::onFingerRelease (int64_t finger, float x, float y)
{
	if (_restart)
		return false;

	const bool motionFinger = _motionFinger;
	_motionFinger = false;
	const uint16_t _x = _frontend->getCoordinateOffsetX() + x * _frontend->getWidth();
	const uint16_t _y = _frontend->getCoordinateOffsetY() + y * _frontend->getHeight();
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		const bool focus = window->checkFocus(_x, _y);
		if (focus && window->onFingerRelease(finger, _x, _y, motionFinger))
			return true;
		if (window->isModal() || window->isFullscreen())
			break;
	}
	return false;
}

bool UI::onFingerPress (int64_t finger, float x, float y)
{
	if (_restart)
		return false;

	const uint16_t _x = _frontend->getCoordinateOffsetX() + x * _frontend->getWidth();
	const uint16_t _y = _frontend->getCoordinateOffsetY() + y * _frontend->getHeight();
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		const bool focus = window->checkFocus(_x, _y);
		if (focus && window->onFingerPress(finger, _x, _y))
			return true;
		if (window->isModal() || window->isFullscreen())
			break;
	}
	return false;
}

void UI::onFingerMotion (int64_t finger, float x, float y, float dx, float dy)
{
	if (_restart)
		return;

	const uint16_t _x = _frontend->getCoordinateOffsetX() + x * _frontend->getWidth();
	const uint16_t _y = _frontend->getCoordinateOffsetY() + y * _frontend->getHeight();
	const int16_t _dx = dx * _frontend->getWidth();
	const int16_t _dy = dy * _frontend->getHeight();
	UIStack stack = _stack;

	const int motionDelta = 10;
	if (_dx > motionDelta || _dy > motionDelta)
		_motionFinger = true;

	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onFingerMotion(finger, _x, _y, _dx, _dy))
			break;
		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	if (_restart)
		return;

	const float speedScale = _mouseSpeed->getFloatValue();
	relX = coordinateScaleX(relX, speedScale, _frontend);
	relY = coordinateScaleY(relY, speedScale, _frontend);
	_frontend->setCursorPosition(_cursorX + relX, _cursorY + relY);
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		const bool focus = window->checkFocus(_cursorX, _cursorY);
		if (focus) {
			window->onMouseMotion(_cursorX, _cursorY, relX, relY);
			break;
		}
		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onMouseButtonRelease (int32_t x, int32_t y, uint8_t button)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onMouseButtonRelease(_cursorX, _cursorY, button))
			break;

		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onMouseButtonPress (int32_t x, int32_t y, uint8_t button)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onMouseButtonPress(_cursorX, _cursorY, button))
			break;
		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onMouseWheel (int32_t x, int32_t y)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onMouseWheel(x, y))
			break;
		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onJoystickDeviceRemoved (int32_t device)
{
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		window->onJoystickDeviceRemoved();
	}
}

void UI::onJoystickMotion (bool horizontal, int v)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onJoystickMotion(horizontal, v))
			return;
		if (window->isModal() || window->isFullscreen())
			break;
	}

	if (Config.getBindingsSpace() != BINDINGS_UI)
		return;

	if (_time - _lastJoystickMoveTime < 150) {
		return;
	}

	// now check whether our value is bigger than our movement delta
	const int delta = 10000;
	static const ICommand::Args args(0);
	if (v < -delta) {
		focusPrev(args);
	} else if (v > delta) {
		focusNext(args);
	}

	_lastJoystickMoveTime = _time;
	_lastJoystickMovementValue = v;
}

void UI::onJoystickButtonPress (uint8_t button)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onJoystickButtonPress(_cursorX, _cursorY, button))
			return;
		if (window->isModal() || window->isFullscreen())
			return;
	}
	Log::debug(LOG_UI, "joystick button %i was pressed and not handled", (int)button);
}

void UI::onMultiGesture (float theta, float dist, int32_t numFingers)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onMultiGesture(theta, dist, numFingers))
			return;
		if (window->isModal() || window->isFullscreen())
			return;
	}
	Log::debug(LOG_UI, "multi gesture event was not handled");
}

void UI::onGesture (int64_t gestureId, float error, int32_t numFingers)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onGesture(gestureId, error, numFingers))
			return;
		if (window->isModal() || window->isFullscreen())
			return;
	}
	Log::debug(LOG_UI, "gesture event was not handled");
}

void UI::onGestureRecord (int64_t gestureId)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onGestureRecord(gestureId))
			return;
		if (window->isModal() || window->isFullscreen())
			return;
	}
	Log::debug(LOG_UI, "gesture record event was not handled");
}

void UI::onControllerButtonPress (const std::string& button)
{
	if (_restart)
		return;

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onControllerButtonPress(_cursorX, _cursorY, button))
			return;
		if (window->isModal() || window->isFullscreen())
			return;
	}
	Log::debug(LOG_UI, "controller button %s was pressed and not handled", button.c_str());
}

UIWindow* UI::getWindow (const std::string& windowID)
{
	return _windows[windowID];
}

void UI::printStack ()
{
	Log::info(LOG_UI, "UI stack");
	for (UIStackReverseIter i = _stack.rbegin(); i != _stack.rend(); ++i) {
		Log::info(LOG_UI, "%s", (*i)->getId().c_str());
	}
}

UIWindow* UI::getRootWindow () const
{
	if (_stack.empty())
		return nullptr;
	return *_stack.begin();
}

bool UI::isMainRoot () const
{
	const UIWindow* window = getRootWindow();
	if (!window)
		return false;
	return window->getId() == UI_WINDOW_MAIN;
}

void UI::pushRoot (const std::string& windowID)
{
	_noPushAllowed = true;
	while (!_stack.empty()) {
		UIWindow* window = *_stack.rbegin();
		window->onPop();
		_stack.pop_back();
		if (!_stack.empty()) {
			_stack.back()->onActive();
		}

		if (window->shouldDelete()) {
			delete window;
		}
	}
	_noPushAllowed = false;
	push(windowID);
}

void UI::pushCmd (const std::string& windowID)
{
	push(windowID);
}

UIWindow* UI::push (const std::string& windowID)
{
	if (_noPushAllowed)
		return nullptr;

	if (!_stack.empty()) {
		UIWindow* activeWindow = *_stack.rbegin();
		if (activeWindow->getId() == windowID) {
			// don't push the same window twice onto
			// the stack just after each other
			return activeWindow;
		}
	}

	UIWindow* window = getWindow(windowID);
	if (!window) {
		Log::error(LOG_UI, "could not find window '%s'", windowID.c_str());
		return nullptr;
	}

	if (_stack.empty() && !window->isRoot())
		return nullptr;

	if (!window->onPush())
		return nullptr;

	Log::info(LOG_UI, "push window %s", windowID.c_str());
	System.track("pushwindow", window->getId());
	if (!_stack.empty()) {
		UIWindow* activeWindow = *_stack.rbegin();
		activeWindow->onPushedOver();
	}
	_stack.push_back(window);
	_stack.back()->onActive();
	return window;
}

void UI::focusNext (const ICommand::Args& args)
{
	if (_stack.empty())
		return;
	if (!_stack.back()->nextFocus(!args.empty()))
		_stack.back()->addFirstFocus();
}

void UI::focusPrev (const ICommand::Args& args)
{
	if (_stack.empty())
		return;
	if (!_stack.back()->prevFocus(!args.empty()))
		_stack.back()->addLastFocus();
}

void UI::runFocusNode ()
{
	if (_stack.empty())
		return;
	_stack.back()->runFocusNode();
}

void UI::pop ()
{
	if (_noPushAllowed)
		return;

	if (_stack.size() == 1) {
		UIPopupCallbackPtr c(new UIPopupOkCommandCallback(CMD_QUIT));
		UI::get().popup(tr("Quit"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
		return;
	}

	UIWindow* window = *_stack.rbegin();
	if (!window->onPop())
		return;

	Log::info(LOG_UI, "pop window %s", window->getId().c_str());
	System.track("popwindow", window->getId());
	_stack.pop_back();
	_stack.back()->onActive();

	if (window->shouldDelete()) {
		delete window;
	}
}

void UI::delayedPop ()
{
	_delayedPop = true;
}

void UI::popMain ()
{
	_noPushAllowed = true;
	// don't pop the root window
	while (_stack.size() > 1) {
		UIWindow* window = *_stack.rbegin();
		if (window->isMain())
			break;
		Log::info(LOG_UI, "pop window %s", window->getId().c_str());
		window->onPop();
		_stack.pop_back();
		if (!_shutdown)
			_stack.back()->onActive();

		if (window->shouldDelete()) {
			delete window;
		}
	}
	_noPushAllowed = false;
}

void UI::popup (const std::string& text, int flags, UIPopupCallbackPtr callback)
{
	Log::info(LOG_UI, "push popup");
	UIWindow* popupWindow = new UIPopupWindow(_frontend, text, flags, callback);
	_stack.push_back(popupWindow);
	_stack.back()->onActive();
}

void UI::getCursorPosition (int &x, int &y) const
{
	x = _cursorX;
	y = _cursorY;
}

void UI::setCursorPosition (int x, int y)
{
	_cursorX = x;
	_cursorY = y;
}

UINodeBar* UI::setBarValue (const std::string& window, const std::string& nodeId, uint16_t value)
{
	UINodeBar* node = getNode<UINodeBar>(window, nodeId);
	if (!node) {
		Log::error(LOG_UI, "could not get the node with the id %s from window %s", nodeId.c_str(), window.c_str());
		return nullptr;
	}
	node->setCurrent(value);
	return node;
}

UINodeBar* UI::setBarMax (const std::string& window, const std::string& nodeId, uint16_t max)
{
	UINodeBar* node = getNode<UINodeBar>(window, nodeId);
	if (!node) {
		Log::error(LOG_UI, "could not get the node with the id %s from window %s", nodeId.c_str(), window.c_str());
		return nullptr;
	}
	node->setMax(max);
	return node;
}
