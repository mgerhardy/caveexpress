#pragma once

#include "engine/client/ui/nodes/UINode.h"
#include "engine/common/Direction.h"
#include "engine/client/IMapControl.h"

class UIWindow;
class ClientMap;

// UI node that implements player controls via mouse, joystick or touch events
class UINodeMapControl: public UINode, public IMapControl {
private:
	ClientMap &_map;
	Direction _direction;
	Direction _oldDirection;
	bool _joystick;

public:
	UINodeMapControl (IFrontend *frontend, ClientMap& map, float x, float y, float w, float h);
	virtual ~UINodeMapControl ();

	// IMapControl
	bool isPressed () const override { return false; }

	// UINode
	void renderDebug (int x, int y, int textY) const override;
	void removeFocus () override;
	bool isActive () const;
	void update (uint32_t deltaTime) override;
	bool onJoystickMotion (bool horizontal, int value) override;
};
