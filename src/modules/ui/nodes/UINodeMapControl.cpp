#include "UINodeMapControl.h"
#include "ui/windows/UIWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "client/ClientMap.h"
#include "common/ConfigManager.h"
#include "common/Log.h"
#include "common/Direction.h"

// TODO:
#define DEVICE_TO_ID(id) (0)

UINodeMapControl::UINodeMapControl (IFrontend *frontend, IUINodeMap *mapNode, bool continuousMovement) :
		UINode(frontend), _map(mapNode->getMap()), _useTriggers(Config.isGameControllerTriggerActive()), _continuousMovement(continuousMovement)
{
	for (int i = 0; i < SDL_arraysize(_d); ++i) {
		_d[i].direction = 0;
		_d[i].oldDirection = 0;
		_d[i].repeated = false;
	}
	setPos(mapNode->getX(), mapNode->getY());
	setSize(mapNode->getWidth(), mapNode->getHeight());
}

UINodeMapControl::~UINodeMapControl ()
{
}

bool UINodeMapControl::isActive () const
{
	return _map.isStarted() && !_map.isPause();
}

void UINodeMapControl::removeFocus ()
{
	UINode::removeFocus();
	for (int i = 0; i < SDL_arraysize(_d); ++i) {
		DirectionValues& dv = _d[i];
		if (dv.direction || dv.oldDirection) {
			_map.resetAcceleration(0, i);
		}
		dv.oldDirection = dv.direction = 0;
		dv.repeated = false;
	}
}

void UINodeMapControl::update (uint32_t deltaTime)
{
	_useTriggers = Config.isGameControllerTriggerActive();
	if (_map.isPause()) {
		for (int i = 0; i < SDL_arraysize(_d); ++i) {
			DirectionValues& dv = _d[i];
			dv.direction = 0;
			dv.repeated = false;
		}
		return;
	}

	UINode::update(deltaTime);

	for (int i = 0; i < SDL_arraysize(_d); ++i) {
		DirectionValues& dv = _d[i];
		if (dv.direction != 0) {
			if (!dv.repeated || _continuousMovement) {
				_map.accelerate(dv.direction, i);
				dv.repeated = true;
			}
		}

		const Direction resetDirections = ~dv.direction & dv.oldDirection;
		if (resetDirections != 0) {
			_map.resetAcceleration(resetDirections, i);
		}

		if (dv.direction == 0)
			dv.repeated = false;

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

void UINodeMapControl::onControllerDeviceAdded (uint32_t id)
{
	Log::info(LOG_UI, "Connect local player with device id %i", id);
}

void UINodeMapControl::onControllerDeviceRemoved (uint32_t id)
{
	Log::info(LOG_UI, "Disonnect local player with device id %i", id);
	DirectionValues& dv = _d[DEVICE_TO_ID(id)];
	dv.direction = 0;
	dv.oldDirection = 0;
}

bool UINodeMapControl::onControllerMotion (uint8_t axis, int value, uint32_t id)
{
	UINode::onControllerMotion(axis, value, id);
	if (_map.isPause()) {
		for (int i = 0; i < SDL_arraysize(_d); ++i) {
			DirectionValues& dv = _d[i];
			dv.direction = 0;
			dv.repeated = false;
		}
		return false;
	}

	const bool horizontal = axis == SDL_CONTROLLER_AXIS_LEFTX || axis == SDL_CONTROLLER_AXIS_RIGHTY;
	if (horizontal)
		Log::trace(LOG_UI, "h controller movement: %i (device %i)", value, id);
	else
		Log::trace(LOG_UI, "v controller movement: %i (device %i)", value, id);

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
		if (_useTriggers) {
			const int absValue = std::abs(value);
			if (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
				if (absValue > delta) {
					dv.direction |= DIRECTION_DOWN;
					dv.direction &= ~DIRECTION_UP;
				} else {
					dv.direction &= ~DIRECTION_VERTICAL;
				}
			} else if (axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
				if (absValue > delta) {
					dv.direction |= DIRECTION_UP;
					dv.direction &= ~DIRECTION_DOWN;
				} else {
					dv.direction &= ~DIRECTION_VERTICAL;
				}
			}
		} else if (value < -delta) {
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
