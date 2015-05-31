#pragma once

#include "client/ui/windows/UIWindow.h"

// forward decl
class ServiceProvider;

class UISettingsWindow: public UIWindow {
protected:
	UINode* _background;
	ServiceProvider& _serviceProvider;
	UINode* addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& option1,
			UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener);
	virtual UINode* addSections();
public:
	UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void init();
};
