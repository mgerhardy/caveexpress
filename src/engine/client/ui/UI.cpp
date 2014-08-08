#include "UI.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "engine/client/ui/BitmapFont.h"
#include "engine/client/ui/windows/UIPopupWindow.h"
#include "engine/client/ui/FontDefinition.h"
#include "engine/common/EventHandler.h"
#include "engine/common/FileSystem.h"
#include "engine/common/IFrontend.h"
#include "engine/common/Logger.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/Commands.h"
#include "engine/common/ConfigManager.h"
#include "engine/client/ui/windows/listener/QuitPopupCallback.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/Singleton.h"
#include "engine/common/FileSystem.h"
#include "engine/GameRegistry.h"
#include "engine/common/gestures/ZoomIn.h"
#include "engine/common/gestures/ZoomOut.h"
#include <SDL.h>
#include <SDL_stdinc.h>

#define GETSCALE_W(x) (x) = (static_cast<float>(x + _frontend->getCoordinateOffsetX()) / _frontend->getWidthScale())
#define GETSCALE_H(y) (y) = (static_cast<float>(y + _frontend->getCoordinateOffsetY()) / _frontend->getHeightScale())

UI::UI () :
		_serviceProvider(nullptr), _eventHandler(nullptr), _frontend(nullptr), _cursor(true), _showCursor(false), _cursorX(
				-1), _cursorY(-1), _restart(false), _delayedPop(false), _noPushAllowed(false), _time(0), _lastJoystickMoveTime(
				0), _lastJoystickMovementValue(0)
{
}

UI::~UI ()
{
	popMain();
}

UI& UI::get ()
{
	static UI ui;
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
	for (UIWindowMapIter i = _windows.begin(); i != _windows.end(); ++i) {
		delete i->second;
	}
	_windows.clear();
	// in case of an error the eventhandler might not yet be set
	if (_eventHandler != nullptr) {
		_eventHandler->removeObserver(this);
		_eventHandler = nullptr;
	}
	_textureCache.shutdown();
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
	const FilePtr& filePtr = FS.getFile(FS.getLanguageDir() + language + ".lang");
	char *buffer;
	int fileLen = filePtr->read((void **) &buffer);
	ScopedArrayPtr<char> p(buffer);
	if (!buffer || fileLen <= 0) {
		error(LOG_CLIENT, "could not load language " + language);
		return false;
	}
	String str(buffer, fileLen);
	std::vector<String> lines = str.split("\n");
	for (std::vector<String>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
		std::vector<String> tuple = i->split("|");
		if (tuple.size() != 2)
			continue;
		_languageMap[tuple[0].str()] = tuple[1].str();
	}
	info(LOG_CLIENT, "loaded language " + language + " with " + string::toString(_languageMap.size()) + " entries");
	return true;
}

const std::string UI::translate (const std::string& in) const
{
	LanguageMap::const_iterator i = _languageMap.find(in);
	if (i == _languageMap.end()) {
		error(LOG_CLIENT, "Missing translation for: " + in);
		return in;
	}

	return i->second;
}

void UI::init (ServiceProvider& serviceProvider, EventHandler &eventHandler, IFrontend &frontend)
{
	const std::string& language = Config.getLanguage();
	if (!initLanguage(language))
		initLanguage("en_GB");
	Commands.registerCommand(CMD_UI_PRINTSTACK, bind(UI, printStack));
	Commands.registerCommand(CMD_UI_PUSH, bind(UI, pushCmd));
	Commands.registerCommand(CMD_UI_RESTART, bind(UI, initRestart));
	Commands.registerCommand(CMD_UI_POP, bind(UI, pop));
	Commands.registerCommand(CMD_UI_FOCUS_NEXT, bind(UI, focusNext));
	Commands.registerCommand(CMD_UI_FOCUS_PREV, bind(UI, focusPrev));
	Commands.registerCommand(CMD_UI_EXECUTE, bind(UI, runFocusNode));
	_showCursor = Config.getConfigVar("showcursor", System.wantCursor() ? "true" : "false", true)->getBoolValue();
	_cursor = _showCursor;
	if (_cursor)
		info(LOG_CLIENT, "enable cursor");
	else
		info(LOG_CLIENT, "disable cursor");

	_serviceProvider = &serviceProvider;
	_eventHandler = &eventHandler;
	_frontend = &frontend;
	eventHandler.registerObserver(this);

	info(LOG_CLIENT, "init the texture cache with " + serviceProvider.getTextureDefinition().getTextureSize());
	_textureCache.init(_frontend, serviceProvider.getTextureDefinition());
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
		const std::string e = SDL_GetError();
		error(LOG_CLIENT, "Failed to load gesture: " + e);
		return false;
	} else if (n == 0) {
		info(LOG_CLIENT, "Could not load gesture");
		return false;
	}

	info(LOG_CLIENT, "Loaded gesture");
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
	_windows[window->getId()] = window;
}

void UI::showCursor (bool show)
{
	// check if the system wants a cursor
	if (!_showCursor) {
		debug(LOG_CLIENT, "ignore show cursor call because the system does not want a cursor");
		return;
	}
	if (show)
		debug(LOG_CLIENT, "show the cursor");
	else
		debug(LOG_CLIENT, "hide the cursor");
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
		const std::string s = String::format("%i:%i", _cursorX, _cursorY);
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

	for (Fonts::iterator i = _fonts.begin(); i != _fonts.end(); ++i) {
		i->second->update(deltaTime);
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

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onKeyPress(key, modifier))
			return true;
		if (window->isModal() || window->isFullscreen())
			break;
	}

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

	const uint16_t _x = _frontend->getCoordinateOffsetX() + x * _frontend->getWidth();
	const uint16_t _y = _frontend->getCoordinateOffsetY() + y * _frontend->getHeight();
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		const bool focus = window->checkFocus(_x, _y);
		if (focus && window->onFingerRelease(finger, _x, _y))
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
		if (window->onFingerPress(finger, _x, _y))
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

	GETSCALE_W(relX);
	GETSCALE_H(relY);
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

	GETSCALE_W(x);
	GETSCALE_H(y);
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onMouseWheel(x, y))
			break;
		if (window->isModal() || window->isFullscreen())
			break;
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

	if (_time - _lastJoystickMoveTime < 350 || horizontal) {
		_lastJoystickMovementValue = v;
		return;
	}

	// skip focus change if we go back to initial position
	if (v > 0 && v < _lastJoystickMovementValue) {
		_lastJoystickMovementValue = v;
		return;
	} else if (v < 0 && v > _lastJoystickMovementValue) {
		_lastJoystickMovementValue = v;
		return;
	}

	// now check whether our value is bigger than our movement delta
	const int delta = 2000;
	if (v < -delta) {
		focusPrev();
	} else if (v > delta) {
		focusNext();
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
	debug(LOG_CLIENT, String::format("joystick button %i was pressed and not handled", (int)button));
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
	debug(LOG_CLIENT, "multi gesture event was not handled");
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
	debug(LOG_CLIENT, "gesture event was not handled");
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
	debug(LOG_CLIENT, "gesture record event was not handled");
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
	debug(LOG_CLIENT, "controller button " + button + " was pressed and not handled");
}

UIWindow* UI::getWindow (const std::string& windowID)
{
	return _windows[windowID];
}

void UI::printStack ()
{
	info(LOG_CLIENT, "UI stack");
	for (UIStackReverseIter i = _stack.rbegin(); i != _stack.rend(); ++i) {
		info(LOG_CLIENT, (*i)->getId());
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
		error(LOG_CLIENT, "could not find window '" + windowID + "'");
		return nullptr;
	}

	if (_stack.empty() && !window->isRoot())
		return nullptr;

	if (!window->onPush())
		return nullptr;

	info(LOG_CLIENT, "push window " + windowID);
	if (!_stack.empty()) {
		UIWindow* activeWindow = *_stack.rbegin();
		activeWindow->onPushedOver();
	}
	_stack.push_back(window);
	_stack.back()->onActive();
	return window;
}

void UI::focusNext ()
{
	if (_stack.empty())
		return;
	if (!_stack.back()->nextFocus())
		_stack.back()->addFirstFocus();
}

void UI::focusPrev ()
{
	if (_stack.empty())
		return;
	if (!_stack.back()->prevFocus())
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
		UIPopupCallbackPtr c(new QuitPopupCallback());
		UI::get().popup(tr("Quit"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
		return;
	}

	UIWindow* window = *_stack.rbegin();
	if (!window->onPop())
		return;

	info(LOG_CLIENT, "pop window " + window->getId());

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
		info(LOG_CLIENT, "pop window " + window->getId());
		window->onPop();
		_stack.pop_back();
		_stack.back()->onActive();

		if (window->shouldDelete()) {
			delete window;
		}
	}
	_noPushAllowed = false;
}

void UI::popup (const std::string& text, int flags, UIPopupCallbackPtr callback)
{
	info(LOG_CLIENT, "push popup");
	UIWindow* popupWindow = new UIPopupWindow(_frontend, text, flags, callback);
	if (popupWindow == nullptr)
		return;
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
		error(LOG_CLIENT, "could not get the node with the id " + nodeId + " from window " + window);
		return nullptr;
	}
	node->setCurrent(value);
	return node;
}

UINodeBar* UI::setBarMax (const std::string& window, const std::string& nodeId, uint16_t max)
{
	UINodeBar* node = getNode<UINodeBar>(window, nodeId);
	if (!node) {
		error(LOG_CLIENT, "could not get the node with the id " + nodeId + " from window " + window);
		return nullptr;
	}
	node->setMax(max);
	return node;
}
