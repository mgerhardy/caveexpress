#pragma once

#include "client/ui/UI.h"
#include "client/ui/windows/UIWindow.h"
#include <string.h>

class UIPopupWindow: public UIWindow {
public:
	UIPopupWindow(IFrontend* frontend, const std::string& text, int flags, UIPopupCallbackPtr callback);

	bool shouldDelete() const override;
};
