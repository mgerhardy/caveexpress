#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class UIMainWindow: public UIWindow {
public:
	UIMainWindow (IFrontend *frontend);

	// UIWindow
	bool onPush () override;
};
