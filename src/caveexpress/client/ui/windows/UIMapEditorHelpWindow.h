#pragma once

#include "UIHelpWindow.h"

class UIMapEditorHelpWindow: public UIHelpWindow {
public:
	UIMapEditorHelpWindow (IFrontend* frontend);
	virtual ~UIMapEditorHelpWindow ();
};
