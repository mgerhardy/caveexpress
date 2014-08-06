#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class UIGestureWindow: public UIWindow {
public:
	UIGestureWindow (IFrontend *frontend);
	void onActive () override;
	bool onGestureRecord (int64_t gestureId) override;
};
