#pragma once

#include "ui/windows/UIHelpWindow.h"

class UIMapEditorHelpWindow: public UIHelpWindow {
public:
	explicit UIMapEditorHelpWindow (IFrontend* frontend);
	virtual ~UIMapEditorHelpWindow ();
};
