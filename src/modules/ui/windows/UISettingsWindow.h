#pragma once

#include "ui/windows/UIWindow.h"
#include "ui/nodes/UINodeLabel.h"

// forward decl
class ServiceProvider;

class UISettingsWindow: public UIWindow {
protected:
	UINode* _background;
	ServiceProvider& _serviceProvider;
	UINode* _controllerNode;
	UINodeLabel* _noController;
	UINode* addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& labelId, const std::string& option1,
			UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener);
	UINode* addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& labelId, const std::string& configVar);
	virtual UINode* addSections();
public:
	UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void init();

	void update(uint32_t time) override;
};
