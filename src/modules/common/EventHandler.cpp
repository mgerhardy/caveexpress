#include "EventHandler.h"
#include "common/Log.h"
#include "common/IEventObserver.h"
#include "common/ConfigManager.h"
#include <SDL.h>

EventHandler::EventHandler () : _multiGesture(false)
{
	//SDL_JoystickEventState(SDL_DISABLE);
	SDL_EventState(SDL_JOYAXISMOTION, SDL_ENABLE);
}

EventHandler::~EventHandler ()
{
}

void EventHandler::registerObserver (IEventObserver* observer)
{
	_observers.push_back(observer);
}

void EventHandler::removeObserver (IEventObserver* observer)
{
	// Traverse through the list and try to find the specified observer
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		if (*i == observer) {
			_observers.erase(i);
			break;
		}
	}
}

inline std::string EventHandler::getControllerButtonName (uint8_t button) const
{
	const char *name = SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(button));
	if (name == nullptr)
		return "unknown";
	return name;
}

bool EventHandler::handleEvent (SDL_Event &event)
{
	switch (event.type) {
	case SDL_TEXTINPUT:
		textInput(std::string(event.text.text));
		break;
	case SDL_KEYUP:
		Log::debug(LOG_COMMON, "release key: %s", SDL_GetScancodeName(event.key.keysym.scancode));
		keyRelease((int32_t) event.key.keysym.sym);
		break;
	case SDL_KEYDOWN:
		// we are handling this on our own
		if (!event.key.repeat) {
			Log::debug(LOG_COMMON, "press key: %s (Key: %i, Mod: %i)",
					SDL_GetScancodeName(event.key.keysym.scancode), event.key.keysym.sym, event.key.keysym.mod);
			keyPress((int32_t) event.key.keysym.sym, (int16_t) event.key.keysym.mod);
		}
		break;
	case SDL_MOUSEMOTION: {
		if (event.motion.which == SDL_TOUCH_MOUSEID)
			break;
		SDL_Window *window = SDL_GetWindowFromID(event.motion.windowID);
		if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS))
			break;
		mouseMotion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
		break;
	}
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.which == SDL_TOUCH_MOUSEID)
			break;
		mouseButtonPress(event.button.x, event.button.y, event.button.button);
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.which == SDL_TOUCH_MOUSEID)
			break;
		mouseButtonRelease(event.button.x, event.button.y, event.button.button);
		break;
	case SDL_MOUSEWHEEL:
		if (event.wheel.which == SDL_TOUCH_MOUSEID)
			break;
		mouseWheel(event.wheel.x, event.wheel.y);
		break;
	case SDL_CONTROLLERAXISMOTION: {
		const uint8_t axis = event.caxis.axis;
		if (axis != SDL_CONTROLLER_AXIS_LEFTX && axis != SDL_CONTROLLER_AXIS_LEFTY
		 && axis != SDL_CONTROLLER_AXIS_RIGHTX && axis != SDL_CONTROLLER_AXIS_RIGHTY)
			break;
		const bool horizontal = (axis == SDL_CONTROLLER_AXIS_LEFTX || axis == SDL_CONTROLLER_AXIS_RIGHTX);
		joystickMotion(horizontal, event.caxis.value);
		break;
	}
	case SDL_CONTROLLERBUTTONDOWN:
		controllerButtonPress(getControllerButtonName(event.cbutton.button));
		break;
	case SDL_CONTROLLERBUTTONUP:
		controllerButtonRelease(getControllerButtonName(event.cbutton.button));
		break;
	case SDL_CONTROLLERDEVICEADDED:
		joystickDeviceAdded(event.cdevice.which);
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		joystickDeviceRemoved(event.cdevice.which);
		break;
	case SDL_JOYDEVICEADDED:
		joystickDeviceAdded(event.jdevice.which);
		break;
	case SDL_JOYDEVICEREMOVED:
		joystickDeviceRemoved(event.jdevice.which);
		break;
	case SDL_DOLLARRECORD:
		gestureRecord(event.dgesture.gestureId);
		break;
	case SDL_DOLLARGESTURE:
		gesture(event.dgesture.gestureId, event.dgesture.error, event.dgesture.numFingers);
		break;
	case SDL_MULTIGESTURE:
		multiGesture(event.mgesture.dTheta, event.mgesture.dDist, event.mgesture.numFingers);
		break;
	case SDL_JOYHATMOTION:
		break;
	case SDL_JOYBUTTONDOWN:
		joystickButtonPress(event.jbutton.button);
		break;
	case SDL_JOYBUTTONUP:
		joystickButtonRelease(event.jbutton.button);
		break;
	case SDL_JOYAXISMOTION:
		joystickMotion(event.jaxis.axis == 0, event.jaxis.value);
		break;
	case SDL_FINGERDOWN:
		fingerPress(event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
		break;
	case SDL_FINGERUP:
		fingerRelease(event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
		break;
	case SDL_FINGERMOTION:
		fingerMotion(event.tfinger.fingerId, event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy);
		break;
	case SDL_WINDOWEVENT:
		switch (event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
				(*i)->onWindowResize();
			}
			break;
		}
		break;
	}
	return true;
}

bool EventHandler::handleAppEvent (SDL_Event &event)
{
	switch (event.type) {
	case SDL_APP_TERMINATING:
		prepareShutdown();
		break;
	case SDL_APP_LOWMEMORY:
		lowMemory();
		break;
	case SDL_APP_WILLENTERBACKGROUND:
		prepareBackground();
		return true;
	case SDL_APP_DIDENTERBACKGROUND:
		background();
		return true;
	case SDL_APP_WILLENTERFOREGROUND:
		prepareForeground();
		return true;
	case SDL_APP_DIDENTERFOREGROUND:
		foreground();
		return true;
	}
	return false;
}

void EventHandler::joystickDeviceAdded (int32_t device)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onJoystickDeviceAdded(device);
	}
}

void EventHandler::joystickDeviceRemoved (int32_t device)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onJoystickDeviceRemoved(device);
	}
}

void EventHandler::lowMemory ()
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onLowMemory();
	}
}

void EventHandler::prepareShutdown ()
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onPrepareShutdown();
	}
}

void EventHandler::prepareBackground ()
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onPrepareBackground();
	}
}

void EventHandler::prepareForeground ()
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onPrepareForeground();
	}
}

void EventHandler::background ()
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onBackground();
	}
}

void EventHandler::foreground ()
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onForeground();
	}
}

void EventHandler::joystickMotion (bool horizontal, int value)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onJoystickMotion(horizontal, value);
	}
}

void EventHandler::mouseWheel (int32_t x, int32_t y)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onMouseWheel(x, y);
	}
}

void EventHandler::mouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onMouseMotion(x, y, relX, relY);
	}
}

void EventHandler::controllerButtonPress (const std::string& button)
{
	if (!Config.isJoystick())
		return;

	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onControllerButtonPress(button);
	}
}

void EventHandler::controllerButtonRelease (const std::string& button)
{
	if (!Config.isJoystick())
		return;

	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onControllerButtonRelease(button);
	}
}

void EventHandler::joystickButtonPress (uint8_t button)
{
	if (!Config.isJoystick())
		return;

	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onJoystickButtonPress(button);
	}
}

void EventHandler::joystickButtonRelease (uint8_t button)
{
	if (!Config.isJoystick())
		return;

	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onJoystickButtonRelease(button);
	}
}

void EventHandler::mouseButtonPress (int32_t x, int32_t y, uint8_t button)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onMouseButtonPress(x, y, button);
	}
}

void EventHandler::mouseButtonRelease (int32_t x, int32_t y, uint8_t button)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onMouseButtonRelease(x, y, button);
	}
}

void EventHandler::textInput (const std::string& text)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onTextInput(text);
	}
}

void EventHandler::keyRelease (int32_t key)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onKeyRelease(key);
	}
}

void EventHandler::keyPress (int32_t key, int16_t modifier)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onKeyPress(key, modifier);
	}
}

void EventHandler::fingerPress (int64_t finger, float x, float y)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onFingerPress(finger, x, y);
	}
}

void EventHandler::fingerRelease (int64_t finger, float x, float y)
{
	_multiGesture = false;
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onFingerRelease(finger, x, y);
	}
}

void EventHandler::fingerMotion (int64_t finger, float x, float y, float dx, float dy)
{
	if (_multiGesture)
		return;
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onFingerMotion(finger, x, y, dx, dy);
	}
}

void EventHandler::gestureRecord (int64_t gestureId)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onGestureRecord(gestureId);
	}
}

void EventHandler::gesture (int64_t gestureId, float error, int32_t numFingers)
{
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onGesture(gestureId, error, numFingers);
	}
}

void EventHandler::multiGesture (float theta, float dist, int32_t numFingers)
{
	_multiGesture = true;
	for (EventObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onMultiGesture(theta, dist, numFingers);
	}
}
