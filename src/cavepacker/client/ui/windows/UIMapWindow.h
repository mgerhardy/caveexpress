#pragma once

#include "ui/windows/IUIMapWindow.h"
#include "ui/nodes/UINodeSlider.h"
#include "cavepacker/client/CavePackerClientMap.h"

namespace cavepacker {

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
	int _targetX;
	int _targetY;

	bool getField (int32_t x, int32_t y, int *tx, int *ty) const;
	void doMove (int tx, int ty);
	bool tryMove (int x, int y, bool doubleTap = true);
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, CavePackerClientMap& map);

	bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override;
	bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y, bool motion) override;
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

}
