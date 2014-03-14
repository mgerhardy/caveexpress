#pragma once

#include "UIMapSelectorWindow.h"

// forward decl
class MapManager;

class UICreateServerWindow: public UIMapSelectorWindow {
public:
	UICreateServerWindow (IFrontend *frontend, const MapManager &mapManager);
	virtual ~UICreateServerWindow ();
};
