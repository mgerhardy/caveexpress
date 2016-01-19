#include "UINodeMapControl.h"
#include "ui/windows/UIWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "client/ClientMap.h"
#include "common/ConfigManager.h"
#include "common/Log.h"
#include "common/Direction.h"

// TODO:
#define DEVICE_TO_ID(id) (0)

UINodeMapControl::UINodeMapControl (IFrontend *frontend, IUINodeMap *mapNode) :
		UINode(frontend), _map(mapNode->getMap()), _joystick(Config.isJoystick())
{
	for (int i = 0; i < SDL_arraysize(_d); ++i) {
		_d[i].direction = 0;
		_d[i].oldDirection = 0;
	}
	setPos(mapNode->getX(), mapNode->getY());
	setSize(mapNode->getWidth(), mapNode->getHeight());
}

UINodeMapControl::~UINodeMapControl ()
{
}

bool UINodeMapControl::isActive () const
{
	return _joystick && _map.isStarted() && !_map.isPause();
}

void UINodeMapControl::removeFocus ()
{
	UINode::removeFocus();
	for (int i = 0; i < SDL_arraysize(_d); ++i) {
		DirectionValues& dv = _d[i];
		if (dv.direction || dv.oldDirection) {
			_map.resetAcceleration(0);
		}
		dv.oldDirection = dv.direction = 0;
	}
}

void UINodeMapControl::update (uint32_t deltaTime)
{
	_joystick = Config.isJoystick();
	if (_map.isPause() || !_joystick) {
		for (int i = 0; i < SDL_arraysize(_d); ++i) {
			DirectionValues& dv = _d[i];
			dv.direction = 0;
		}
		return;
	}

	UINode::update(deltaTime);

	for (int i = 0; i < SDL_arraysize(_d); ++i) {
		DirectionValues& dv = _d[i];
		if (dv.direction != 0) {
			_map.accelerate(dv.direction);
		}

		const Direction resetDirections = ~dv.direction & dv.oldDirection;
		if (resetDirections != 0) {
			_map.resetAcceleration(resetDirections);
		}

		dv.oldDirection = dv.direction;
	}
}

void UINodeMapControl::renderDebug (int x, int y, int textY) const
{
	UINode::renderDebug(x, y, textY);

	const int cx = getRenderCenterX();
	const int cy = getRenderCenterY();

	const int width = 80;
	for (int i = 0; i < SDL_arraysize(_d); ++i) {
		const DirectionValues& dv = _d[i];
		if (dv.direction & DIRECTION_LEFT) {
			renderLine(cx, cy, cx - width, cy, colorGreen);
		}
		if (dv.direction & DIRECTION_RIGHT) {
			renderLine(cx, cy, cx + width, cy, colorBrightGreen);
		}
		if (dv.direction & DIRECTION_UP) {
			renderLine(cx, cy, cx, cy - width, colorBlue);
		}
		if (dv.direction & DIRECTION_DOWN) {
			renderLine(cx, cy, cx, cy + width, colorBrightBlue);
		}
	}
}

void UINodeMapControl::onJoystickDeviceAdded (uint32_t id)
{
	Log::info(LOG_UI, "Connect local player with device id %i", id);
}

void UINodeMapControl::onJoystickDeviceRemoved (uint32_t id)
{
	Log::info(LOG_UI, "Disonnect local player with device id %i", id);
	DirectionValues& dv = _d[DEVICE_TO_ID(id)];
	dv.direction = 0;
	dv.oldDirection = 0;
}

bool UINodeMapControl::onJoystickMotion (bool horizontal, int value, uint32_t id)
{
	UINode::onJoystickMotion(horizontal, value, id);
	if (_map.isPause() || !_joystick) {
		for (int i = 0; i < SDL_arraysize(_d); ++i) {
			DirectionValues& dv = _d[i];
			dv.direction = 0;
		}
		return false;
	}

	if (horizontal)
		Log::trace(LOG_UI, "h joystick movement: %i (device %i)", value, id);
	else
		Log::trace(LOG_UI, "v joystick movement: %i (device %i)", value, id);

	const int delta = 8000;
	DirectionValues& dv = _d[DEVICE_TO_ID(id)];
	if (horizontal) {
		if (value < -delta) {
			dv.direction |= DIRECTION_LEFT;
			dv.direction &= ~DIRECTION_RIGHT;
		} else if (value > delta) {
			dv.direction |= DIRECTION_RIGHT;
			dv.direction &= ~DIRECTION_LEFT;
		} else {
			dv.direction &= ~DIRECTION_HORIZONTAL;
		}
	} else {
		if (value < -delta) {
			dv.direction |= DIRECTION_UP;
			dv.direction &= ~DIRECTION_DOWN;
		} else if (value > delta) {
			dv.direction |= DIRECTION_DOWN;
			dv.direction &= ~DIRECTION_UP;
		} else {
			dv.direction &= ~DIRECTION_VERTICAL;
		}
	}

	return true;
}
