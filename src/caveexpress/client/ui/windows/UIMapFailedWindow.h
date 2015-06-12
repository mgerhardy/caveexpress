#pragma once

#include "ui/windows/UIWindow.h"
#include "common/MapFailedReason.h"
#include "common/ThemeType.h"

// forward decl
class CampaignManager;
class UINodeButton;

namespace caveexpress {

class UINodeBackgroundScene;

class UIMapFailedWindow: public UIWindow {
private:
	UINodeBackgroundScene *_background;
	CampaignManager& _campaignManager;
	UINodeButton *_replayCampaign;
public:
	UIMapFailedWindow (IFrontend *frontend, CampaignManager& campaignManager);
	void updateReason (bool isMultiplayer, const MapFailedReason& reason, const ThemeType& theme);
};

}
