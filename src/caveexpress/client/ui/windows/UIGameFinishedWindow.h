#pragma once

#include "ui/windows/UIWindow.h"

class UIGameFinishedWindow: public UIWindow {
public:
	explicit UIGameFinishedWindow (IFrontend *frontend);
};
