#pragma once

#include "UIMapSelectorWindow.h"

// forward decl
class IMapManager;

class UICreateServerWindow: public UIMapSelectorWindow {
private:
	class UINodeSpinner* _maxPlayers;
public:
	UICreateServerWindow (IFrontend *frontend, const IMapManager &mapManager);
	virtual ~UICreateServerWindow ();

	void onActive () override;
};
