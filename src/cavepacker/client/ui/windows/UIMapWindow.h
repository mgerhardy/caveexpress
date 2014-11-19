#pragma once

#include "engine/client/ui/windows/IUIMapWindow.h"
#include "engine/client/ui/nodes/UINodeSlider.h"

class UICavePackerNodePoint;

class UIMapWindow: public IUIMapWindow {
protected:
	UINode* getFingerControl () override;
	void initHudNodes () override;
	void initInputHudNodes () override;
	UINode* _undo;
	UINodeSlider* _autoSolveSlider;
	UICavePackerNodePoint* _points;
	CampaignManager& _campaignManager;
	bool _scrolling;
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);

	bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override;
	bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button) override;
	bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button) override;
	void onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override;

	void initWaitingForPlayers (bool adminOptions) override;
	void hideHud() override;
	void showHud() override;
	void showAutoSolveSlider();
	void hideAutoSolveSlider();
	void showCursor (bool show) override;
};
