#pragma once

#include "client/ui/nodes/UINodeSelector.h"
#include "common/campaign/Campaign.h"

class CampaignManager;

class UINodeCampaignSelector: public UINodeBackgroundSelector<CampaignPtr>, public ICampaignVisitor {
private:
	CampaignManager &_campaignManager;

	// ICampaignVisitor
	void visitCampaign (CampaignPtr& campaign);
public:
	UINodeCampaignSelector (IFrontend *frontend, CampaignManager &campaignManager, int cols, int rows);
	virtual ~UINodeCampaignSelector ();

	// UINodeSelector
	bool onSelect (const CampaignPtr& data) override;
	void renderSelectorEntry (int index, const CampaignPtr& data, int x, int y, int colWidth, int rowHeight, float alpha) const override;
	void reset () override;
};
