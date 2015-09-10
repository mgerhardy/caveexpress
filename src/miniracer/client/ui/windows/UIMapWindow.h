#pragma once

#include "ui/windows/IUIMapWindow.h"
#include "ui/nodes/UINodeSlider.h"
#include "ui/nodes/UINodePoint.h"
#include "miniracer/client/MiniRacerClientMap.h"

namespace miniracer {

class UIMapWindow: public IUIMapWindow {
protected:
	UINode* getFingerControl () override;
	void initHudNodes () override;
	void initInputHudNodes () override;
	UINode* _undo;
	UINodePoint* _points;
	CampaignManager& _campaignManager;
	bool _scrolling;
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, MiniRacerClientMap& map);

	bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override;
	bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button) override;
	bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button) override;
	void onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override;

	void hideHud() override;
	void showHud() override;
	void showCursor (bool show) override;
};

}
