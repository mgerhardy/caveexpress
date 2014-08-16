#pragma once

#include "engine/client/ui/windows/IUIMapWindow.h"
#include "engine/client/ui/nodes/UINodeSlider.h"

class UICaveAndDiamondsNodePoint;

class UIMapWindow: public IUIMapWindow {
protected:
	UINode* getFingerControl () override;
	void initHudNodes () override;
	void initInputHudNodes () override;
	UINode* _undo;
	UINodeSlider* _autoSolveSlider;
	UICaveAndDiamondsNodePoint* _points;
	CampaignManager& _campaignManager;
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);

	void initWaitingForPlayers (bool adminOptions) override;
	void hideHud() override;
	void showHud() override;
	void showAutoSolveSlider();
	void hideAutoSolveSlider();
	void showCursor (bool show) override;
};
