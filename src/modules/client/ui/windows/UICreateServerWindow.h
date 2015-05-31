#pragma once

#include "UIMapSelectorWindow.h"

// forward decl
class IMapManager;

class UICreateServerWindow: public UIMapSelectorWindow {
public:
	UICreateServerWindow (IFrontend *frontend, const IMapManager &mapManager);
	virtual ~UICreateServerWindow ();
};
