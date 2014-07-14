#pragma once

#include "engine/client/ui/windows/UIWindow.h"

#define UINODE_SERVERSELECTOR "servers"

// forward decl
class IMapManager;
class ServiceProvider;

class UIMultiplayerWindow: public UIWindow {
private:
	ServiceProvider& _serviceProvider;
public:
	UIMultiplayerWindow (IFrontend *frontend, const IMapManager &mapManager, ServiceProvider& serviceProvider);
	virtual ~UIMultiplayerWindow ();

	void onActive () override;
	bool onPop () override;
};
