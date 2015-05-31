#pragma once

#include "client/ui/windows/UIWindow.h"

// forward decl
class UINodeButton;
class CampaignManager;

class UIModeSelectionWindow: public UIWindow {
private:
	UINodeButton *_easyMode;
	UINodeButton *_hardMode;
public:
	UIModeSelectionWindow (IFrontend *frontend, CampaignManager& campaignManager);

	bool onPop () override;
};
