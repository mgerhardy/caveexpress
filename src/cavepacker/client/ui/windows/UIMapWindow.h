#pragma once

#include "engine/client/ui/windows/IUIMapWindow.h"
#include "engine/client/ui/nodes/UINodeSlider.h"

class UIMapWindow: public IUIMapWindow {
protected:
	UINode* getFingerControl () override;
	void initHudNodes () override;
	UINode* _undo;
	UINodeSlider* _autoSolveSlider;
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);

	void hideHud() override;
	void showHud() override;
	void showAutoSolveSlider();
	void hideAutoSolveSlider();
};
