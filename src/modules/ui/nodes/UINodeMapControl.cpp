#include "UINodeMapControl.h"
#include "ui/windows/UIWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "client/ClientMap.h"
#include "common/ConfigManager.h"
#include "common/Log.h"
#include "common/Direction.h"


UINodeMapControl::UINodeMapControl (IFrontend *frontend, IUINodeMap *mapNode) :
		UINode(frontend), _map(mapNode->getMap()), _direction(0), _oldDirection(0), _joystick(Config.isJoystick())
{
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
	if (_direction || _oldDirection) {
		_map.resetAcceleration(0);
	}
	_oldDirection = _direction = 0;
}

void UINodeMapControl::update (uint32_t deltaTime)
{
	_joystick = Config.isJoystick();
	if (_map.isPause() || !_joystick) {
		_direction = 0;
		return;
	}

	UINode::update(deltaTime);

	if (_direction != 0) {
		_map.accelerate(_direction);
	}

	const Direction resetDirections = ~_direction & _oldDirection;
	if (resetDirections != 0) {
		_map.resetAcceleration(resetDirections);
	}

	_oldDirection = _direction;
}

void UINodeMapControl::renderDebug (int x, int y, int textY) const
{
	UINode::renderDebug(x, y, textY);

	const int cx = getRenderCenterX();
	const int cy = getRenderCenterY();

	const int width = 80;
	if (_direction & DIRECTION_LEFT) {
		renderLine(cx, cy, cx - width, cy, colorGreen);
	}
	if (_direction & DIRECTION_RIGHT) {
		renderLine(cx, cy, cx + width, cy, colorBrightGreen);
	}
	if (_direction & DIRECTION_UP) {
		renderLine(cx, cy, cx, cy - width, colorBlue);
	}
	if (_direction & DIRECTION_DOWN) {
		renderLine(cx, cy, cx, cy + width, colorBrightBlue);
	}
}

bool UINodeMapControl::onJoystickMotion (bool horizontal, int value)
{
	UINode::onJoystickMotion(horizontal, value);
	if (_map.isPause() || !_joystick) {
		_direction = 0;
		return false;
	}

	if (horizontal)
		Log::info(LOG_UI, "h joystick movement: %i", value);
	else
		Log::info(LOG_UI, "v joystick movement: %i", value);

	const int delta = 8000;
	if (horizontal) {
		if (value < -delta) {
			_direction |= DIRECTION_LEFT;
			_direction &= ~DIRECTION_RIGHT;
		} else if (value > delta) {
			_direction |= DIRECTION_RIGHT;
			_direction &= ~DIRECTION_LEFT;
		} else {
			_direction &= ~DIRECTION_HORIZONTAL;
		}
	} else {
		if (value < -delta) {
			_direction |= DIRECTION_UP;
			_direction &= ~DIRECTION_DOWN;
		} else if (value > delta) {
			_direction |= DIRECTION_DOWN;
			_direction &= ~DIRECTION_UP;
		} else {
			_direction &= ~DIRECTION_VERTICAL;
		}
	}

	return true;
}
