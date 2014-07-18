#pragma once

#include "UIMapSelectorWindow.h"

// forward decl
class CampaignManager;

class UICampaignMapWindow: public UIMapSelectorWindow {
public:
	UICampaignMapWindow (IFrontend *frontend, CampaignManager &campaignManager);
};
