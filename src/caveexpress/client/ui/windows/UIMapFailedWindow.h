#pragma once

#include "engine/client/ui/windows/UIWindow.h"
#include "engine/common/MapFailedReason.h"
#include "engine/common/ThemeType.h"

// forward decl
class CampaignManager;
class UINodeBackgroundScene;
class UINodeButton;

class UIMapFailedWindow: public UIWindow {
private:
	UINodeBackgroundScene *_background;
	CampaignManager& _campaignManager;
	UINodeButton *_replayCampaign;
public:
	UIMapFailedWindow (IFrontend *frontend, CampaignManager& campaignManager);
	void updateReason (bool isMultiplayer, const MapFailedReason& reason, const ThemeType& theme);
};
