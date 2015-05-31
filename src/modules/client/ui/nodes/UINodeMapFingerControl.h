#pragma once

#include "client/ui/nodes/UINode.h"
#include "common/Direction.h"
#include "client/IMapControl.h"

class UIWindow;
class ClientMap;
class IUINodeMap;

// UI node that implements player controls via mouse, joystick or touch events
class UINodeMapFingerControl: public UINode, public IMapControl {
private:
	ClientMap &_map;
	int64_t _finger;
	int _pressX;
	int _pressY;
	int _moveX;
	int _moveY;
	int _lastMoveX;
	int _lastMoveY;

public:
	UINodeMapFingerControl (IFrontend *frontend, IUINodeMap *mapNode);
	virtual ~UINodeMapFingerControl ();

	// IMapControl
	bool isPressed () const override
	{
		return _finger != -1;
	}
	void hide () override { setVisible(false); }
	void show () override { setVisible(true); }

	// UINode
	void removeFocus () override;
	bool isActive () const override;
	void update (uint32_t deltaTime) override;
	bool onPush () override;
	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;
	bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y) override;
	bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override;
};
