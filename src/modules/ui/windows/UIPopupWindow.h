#pragma once

#include "ui/UI.h"
#include "ui/windows/UIWindow.h"
#include <string.h>

class UIPopupWindow: public UIWindow {
public:
	UIPopupWindow(IFrontend* frontend, const std::string& text, int flags, UIPopupCallbackPtr callback);

	bool shouldDelete() const override;
};
