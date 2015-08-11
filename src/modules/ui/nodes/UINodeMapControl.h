#pragma once

#include "ui/nodes/UINode.h"
#include "common/Direction.h"
#include "client/IMapControl.h"

class UIWindow;
class ClientMap;
class IUINodeMap;

// UI node that implements player controls via joystick or touch events
class UINodeMapControl: public UINode, public IMapControl {
private:
	ClientMap &_map;
	Direction _direction;
	Direction _oldDirection;
	bool _joystick;

public:
	UINodeMapControl (IFrontend *frontend, IUINodeMap *mapNode);
	virtual ~UINodeMapControl ();

	// IMapControl
	bool isPressed () const override { return false; }
	void hide () override { setVisible(false); }
	void show () override { setVisible(true); }

	// UINode
	void renderDebug (int x, int y, int textY) const override;
	void removeFocus () override;
	bool isActive () const override;
	void update (uint32_t deltaTime) override;
	bool onJoystickMotion (bool horizontal, int value) override;
};
