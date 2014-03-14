#pragma once

#include "engine/client/ui/windows/UIWindow.h"

// forward decl
class ServiceProvider;

class UIMapOptionsWindow: public UIWindow {
private:
	UINode *_restartMap;
	ServiceProvider& _serviceProvider;
public:
	UIMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void update (uint32_t deltaTime) override;
};
