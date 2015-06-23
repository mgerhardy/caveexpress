#include "UINodeMapFingerControl.h"
#include "ui/windows/UIWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "client/ClientMap.h"
#include "common/ConfigManager.h"
#include "common/Log.h"

UINodeMapFingerControl::UINodeMapFingerControl (IFrontend *frontend, IUINodeMap *mapNode) :
		UINode(frontend), _map(mapNode->getMap()), _finger(-1), _pressX(0), _pressY(0), _moveX(0), _moveY(0), _lastMoveX(0), _lastMoveY(0)
{
	setPos(mapNode->getX(), mapNode->getY());
	setSize(mapNode->getWidth(), mapNode->getHeight());
}

UINodeMapFingerControl::~UINodeMapFingerControl ()
{
}

void UINodeMapFingerControl::removeFocus ()
{
	UINode::removeFocus();
	_finger = -1;
	_lastMoveX = _lastMoveY = _moveX = _moveY = 0;
	_map.resetAcceleration();
}

bool UINodeMapFingerControl::onPush ()
{
	_finger = -1;
	_lastMoveX = _lastMoveY = _moveX = _moveY = 0;
	return UINode::onPush();
}

void UINodeMapFingerControl::update (uint32_t deltaTime)
{
	UINode::update(deltaTime);
	if (!isPressed()) {
		return;
	}

	if (_lastMoveX == _moveX && _lastMoveY == _moveY) {
		return;
	}

	_map.setAcceleration(_moveX, _moveY);
	_lastMoveX = _moveX;
	_lastMoveY = _moveY;
	_moveX = _moveY = 0;
}

bool UINodeMapFingerControl::isActive () const
{
	return _map.isStarted() && !_map.isPause();
}

bool UINodeMapFingerControl::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	UINode::onFingerPress(finger, x, y);

	if (_map.isPause())
		return false;

	if (isPressed()) {
		Log::debug2(LOG_CLIENT, "pressing second finger");
		return _map.secondFinger();
	}

	_lastMoveX = _lastMoveY = -1;
	_moveX = _moveY = 0;
	_finger = finger;
	_pressX = x;
	_pressY = y;

	return true;
}

bool UINodeMapFingerControl::onFingerRelease (int64_t finger, uint16_t x, uint16_t y)
{
	const bool val = UINode::onFingerRelease(finger, x, y);

	if (_finger == finger) {
		_finger = -1;
		_lastMoveX = _lastMoveY = _moveX = _moveY = 0;
		_map.resetAcceleration();
		return true;
	}

	return val;
}

bool UINodeMapFingerControl::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
	UINode::onFingerMotion(finger, x, y, dx, dy);
	if (_map.isPause()) {
		return false;
	}

	if (_finger != finger)
		return false;

	_moveX = dx;
	_moveY = dy;

	return true;
}
