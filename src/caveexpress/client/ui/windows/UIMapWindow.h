#pragma once

#include "ui/windows/IUIMapWindow.h"

class UINodeLabel;

namespace caveexpress {

class UIMapWindow: public IUIMapWindow {
private:
	UINodeLabel* _spectateLabel = nullptr;
public:
	UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map);
	virtual void initHudNodes() override;
	void update (uint32_t deltaTime) override;
	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;
};

}
