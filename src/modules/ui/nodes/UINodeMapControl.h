#pragma once

#include "ui/nodes/UINode.h"
#include "common/Direction.h"
#include "client/IMapControl.h"

class UIWindow;
class ClientMap;
class IUINodeMap;

// UI node that implements player controls via controller events
class UINodeMapControl: public UINode, public IMapControl {
private:
	ClientMap &_map;
	struct DirectionValues {
		Direction direction;
		Direction oldDirection;
	};
	DirectionValues _d[4];
	bool _useTriggers;

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
	bool onControllerMotion (uint8_t axis, int value, uint32_t id) override;
	void onControllerDeviceRemoved (uint32_t id) override;
	void onControllerDeviceAdded (uint32_t id) override;
};
