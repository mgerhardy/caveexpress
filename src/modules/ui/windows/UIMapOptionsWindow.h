#pragma once

#include "ui/windows/UIWindow.h"

// forward decl
class ServiceProvider;
class UINodeBackButton;

class UIMapOptionsWindow: public UIWindow {
protected:
	UINode *_restartMap;
	ServiceProvider& _serviceProvider;
	UINodeBackButton *_backButton;
public:
	UIMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void update (uint32_t deltaTime) override;

	virtual void onActive () override;
	bool onPop () override;
};
