#pragma once

#include "UIWindow.h"

class UIMainWindow: public UIWindow {
public:
	UIMainWindow (IFrontend *frontend);
	virtual ~UIMainWindow ();
};
