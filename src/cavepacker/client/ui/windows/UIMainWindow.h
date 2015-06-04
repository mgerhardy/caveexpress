#pragma once

#include "ui/windows/UIWindow.h"

class UIMainWindow: public UIWindow {
public:
	explicit UIMainWindow (IFrontend *frontend);
};
