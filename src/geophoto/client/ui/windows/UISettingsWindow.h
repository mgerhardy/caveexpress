#pragma once

#include "UIWindow.h"

class UISettingsWindow: public UIWindow {
public:
	UISettingsWindow (IFrontend *frontend);
	virtual ~UISettingsWindow ();
};
