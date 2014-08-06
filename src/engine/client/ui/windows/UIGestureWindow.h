#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class UIGestureWindow: public UIWindow {
public:
	UIGestureWindow (IFrontend *frontend);
	void onActive () override;
	bool onGestureRecord (int64_t gestureId) override;
	bool onGesture (int64_t gestureId) override;
	bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override;
};
