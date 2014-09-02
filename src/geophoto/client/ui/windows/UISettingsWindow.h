#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class UISettingsWindow: public UIWindow {
public:
	UISettingsWindow (IFrontend *frontend);
	virtual ~UISettingsWindow ();
};
