#pragma once

#include "UIHelpWindow.h"

class UIMapEditorHelpWindow: public UIHelpWindow {
public:
	explicit UIMapEditorHelpWindow (IFrontend* frontend);
	virtual ~UIMapEditorHelpWindow ();
};
