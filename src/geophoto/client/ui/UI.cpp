#include "UI.h"
#include "client/ui/FontDefinition.h"
#include "client/ui/windows/UIMainWindow.h"
#include "client/ui/windows/UIMapWindow.h"
#include "client/ui/windows/UISettingsWindow.h"
#include "client/ui/windows/UIRoundResultWindow.h"
#include "client/ui/windows/UIViewResultWindow.h"
#include "client/ui/nodes/UINodeBar.h"
#include "client/round/RoundController.h"
#include "client/ui/BitmapFont.h"
#include "shared/EventHandler.h"
#include "shared/FileSystem.h"
#include "shared/IFrontend.h"
#include "shared/Logger.h"
#include "shared/CommandSystem.h"
#include "shared/ConfigManager.h"
#include "shared/ServiceProvider.h"
#include "common/Singleton.h"
#include <SDL.h>

#define GETSCALE_W(x) (x) = (static_cast<float>(x + _frontend->getCoordinateOffsetX()) / _frontend->getWidthScale())
#define GETSCALE_H(y) (y) = (static_cast<float>(y + _frontend->getCoordinateOffsetY()) / _frontend->getHeightScale())

UI::UI () :
		_eventHandler(nullptr), _frontend(nullptr), _roundController(nullptr), _cursorX(-1), _cursorY(-1), _restart(false), _serviceProvider(nullptr)
{
	_cursor = Config.getConfigVar("showcursor", "true", true);
}

UI::~UI ()
{
	shutdown();
	if (_roundController)
		delete _roundController;
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

	std::string root = getRootWindow()->getName();

	shutdown();
	init(*serviceProvider, *eventHandler, *frontend);

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

	_stack.clear();
	_fonts.clear();

	Commands.removeCommand(CMD_UI_PRINTSTACK);
	Commands.removeCommand(CMD_UI_PUSH);
	Commands.removeCommand(CMD_UI_POP);
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

void UI::init (ServiceProvider& serviceProvider, EventHandler &eventHandler, IFrontend &frontend)
{
	Commands.registerCommand(CMD_UI_PRINTSTACK, bind(UI, printStack));
	Commands.registerCommand(CMD_UI_PUSH, bind(UI, push));
	Commands.registerCommand(CMD_UI_POP, bind(UI, pop));
	Commands.registerCommand(CMD_UI_FOCUS_NEXT, bind(UI, focusNext));
	Commands.registerCommand(CMD_UI_EXECUTE, bind(UI, execute));
	_cursor = Config.getConfigVar("showcursor", "true", true);

	_serviceProvider = &serviceProvider;
	_eventHandler = &eventHandler;
	_frontend = &frontend;
	eventHandler.registerObserver(this);
	_roundController = new RoundController(serviceProvider.getGameStatePersister(), this);

	info(LOG_CLIENT, "init the texture cache");
	_textureCache.init(_frontend, serviceProvider);

	FontDefinition& fontDef = Singleton<FontDefinition>::getInstance();
	FontDefMapConstIter i = fontDef.begin();
	for (; i != fontDef.end(); ++i) {
		_fonts[i->second->id] = BitmapFontPtr(new BitmapFont(i->second, _frontend));
	}

	addWindow(new UIMainWindow(&frontend));
	addWindow(new UISettingsWindow(&frontend));
	addWindow(new UIMapWindow(&frontend, *_roundController));
	addWindow(new UIRoundResultWindow(&frontend, _roundController->getGameRound()));
	addWindow(new UIViewResultWindow(&frontend, _roundController->getGameRound()));

	_mouseCursor = loadTexture("mouse");
}

void UI::initStack ()
{
	if (!_stack.empty())
		return;

	// push the main window onto the stack
	push(UI_WINDOW_MAIN);
}

void UI::addWindow (UIWindow *window)
{
	_windows[window->getName()] = window;
}

void UI::showCursor (bool show)
{
	_cursor->setValue(show ? "true" : "false");
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
	TexturePtr textureLoaded = loadTexture("bubble-full");
	TexturePtr textureNotLoaded = loadTexture("bubble");
	TexturePtr loading = loadTexture("loading");
	const int textureWidth = textureLoaded->getWidth();
	const int textureHeight = textureLoaded->getHeight();
	const int gap = textureHeight / 2;
	const int x = _frontend->getWidth() / 2 - textureWidth * 5 - gap * 4.5;
	const int y = _frontend->getHeight() / 2 - textureHeight / 2;
	const Color color = { 0.0f, 0.0f, 0.0f, 1.0f };
	_cursor->setValue("false");
	_frontend->renderFilledRect(0, 0, _frontend->getWidth(), _frontend->getHeight(), color);

	if (_progress.steps > 0 && _progress.step > 0) {
		const float factor = static_cast<float>(_progress.step) / static_cast<float>(_progress.steps);
		const int percent = factor * 100.0f;
		const int loaded = percent / 10;
		_frontend->renderImage(loading.get(), _frontend->getWidth() / 2 - loading->getWidth() / 2, y - loading->getHeight() - (gap * 2), loading->getWidth(), loading->getHeight(), 0, 1.0f);
		for (int i = 0; i < 10; ++i) {
			if (i < loaded || loaded == 10) {
				_frontend->renderImage(textureLoaded.get(), x + i * textureWidth + i * gap, y, textureWidth, textureHeight, 0, 1.0f);
			} else {
				_frontend->renderImage(textureNotLoaded.get(), x + i * textureWidth + i * gap, y, textureWidth, textureHeight, 0, 1.0f);
			}
		}
		if (loaded == 10) {
			_cursor->setValue("true");
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
		(*i)->render();
	}

	// TODO: move cursor and progress bar into frontend
	if (_progress.active) {
		renderProgress();
	}

	if (_cursorX != -1 && _cursorY != -1 && _cursor->getBoolValue()) {
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
	if (_restart)
		restart();

	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		window->update(deltaTime);
	}
}

bool UI::onKeyRelease (int32_t key)
{
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

bool UI::onFingerRelease (int64_t finger, float x, float y)
{
	const uint16_t _x = _frontend->getCoordinateOffsetX() + x * _frontend->getWidth();
	const uint16_t _y = _frontend->getCoordinateOffsetY() + y * _frontend->getHeight();
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onFingerRelease(finger, _x, _y))
			return true;

		if (window->isModal() || window->isFullscreen())
			break;
	}
	return false;
}

bool UI::onFingerPress (int64_t finger, float x, float y)
{
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
	GETSCALE_W(relX);
	GETSCALE_H(relY);
	_frontend->setCursorPosition(_cursorX + relX, _cursorY + relY);
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		if (window->onMouseMotion(_cursorX, _cursorY, relX, relY))
			break;
		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onMouseButtonRelease (int32_t x, int32_t y, uint8_t button)
{
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
	GETSCALE_W(x);
	GETSCALE_H(y);
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		window->onMouseWheel(x, y);

		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onJoystickMotion (bool horizontal, float v)
{
	// convert the relative movement into pixel movement
	int value;
	if (horizontal) {
		value = v * _frontend->getWidth() / 8;
		_frontend->setCursorPosition(_cursorX + value, _cursorY);
	} else {
		value = v * _frontend->getHeight() / 8;
		_frontend->setCursorPosition(_cursorX, _cursorY + value);
	}
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		window->onJoystickMotion(_cursorX, _cursorY, horizontal, value);

		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onJoystickButtonPress (uint8_t button)
{
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		window->onJoystickButtonPress(_cursorX, _cursorY, button);

		if (window->isModal() || window->isFullscreen())
			break;
	}
}

void UI::onControllerButtonPress (const std::string& button)
{
	UIStack stack = _stack;
	for (UIStackReverseIter i = stack.rbegin(); i != stack.rend(); ++i) {
		UIWindow* window = *i;
		window->onControllerButtonPress(_cursorX, _cursorY, button);

		if (window->isModal() || window->isFullscreen())
			break;
	}
}

UIWindow* UI::getWindow (const std::string& windowID)
{
	return _windows[windowID];
}

void UI::printStack ()
{
	info(LOG_CLIENT, "UI stack");
	for (UIStackReverseIter i = _stack.rbegin(); i != _stack.rend(); ++i) {
		info(LOG_CLIENT, (*i)->getName());
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
	return window->getName() == UI_WINDOW_MAIN;
}

void UI::pushRoot (const std::string& windowID)
{
	_stack.clear();
	push(windowID);
}

void UI::push (const std::string& windowID)
{
	if (!_stack.empty()) {
		UIWindow* activeWindow = *_stack.rbegin();
		if (activeWindow->getName() == windowID) {
			// don't push the same window twice onto
			// the stack just after each other
			return;
		}
	}

	UIWindow* window = getWindow(windowID);
	if (!window) {
		error(LOG_CLIENT, "could not find window '" + windowID + "'");
		return;
	}

	if (_stack.empty() && !window->isRoot())
		return;

	if (!window->onPush())
		return;

	info(LOG_CLIENT, "push window " + windowID);
	_stack.push_back(window);
	_stack.back()->checkFocus(_cursorX, _cursorY);
	_stack.back()->onActive();
}

void UI::focusNext ()
{
	if (_stack.empty())
		return;
	_stack.back()->nextFocus();
}

void UI::execute ()
{
	if (_stack.empty())
		return;
	_stack.back()->execute();
}

void UI::pop ()
{
	if (_stack.size() == 1) {
		return;
	}

	UIWindow* window = *_stack.rbegin();
	if (!window->onPop())
		return;

	info(LOG_CLIENT, "pop window " + window->getName());
	_stack.pop_back();
	_stack.back()->checkFocus(_cursorX, _cursorY);
	_stack.back()->onActive();
}

void UI::popMain ()
{
	// don't pop the root window
	while (_stack.size() > 1) {
		const UIWindow* window = *_stack.rbegin();
		if (window->isMain())
			break;
		info(LOG_CLIENT, "pop window " + window->getName());
		_stack.pop_back();
	}
	_stack.back()->checkFocus(_cursorX, _cursorY);
}

void UI::setCursorPosition (int x, int y)
{
	_cursorX = x;
	_cursorY = y;
}

void UI::setBarValue (const std::string& window, const std::string& nodeId, uint16_t value)
{
	UINodeBar* node = getNode<UINodeBar>(window, nodeId);
	if (!node) {
		error(LOG_CLIENT, "could not get the node with the id " + nodeId + " from window " + window);
		return;
	}
	node->setCurrent(value);
}

void UI::setBarMax (const std::string& window, const std::string& nodeId, uint16_t max)
{
	UINodeBar* node = getNode<UINodeBar>(window, nodeId);
	if (!node) {
		error(LOG_CLIENT, "could not get the node with the id " + nodeId + " from window " + window);
		return;
	}
	node->setMax(max);
}
