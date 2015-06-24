#include "UINodeCampaignSelector.h"
#include "campaign/CampaignManager.h"
#include "common/CommandSystem.h"
#include "ui/UI.h"

UINodeCampaignSelector::UINodeCampaignSelector (IFrontend *frontend, CampaignManager &campaignManager, int cols, int rows) :
		UINodeBackgroundSelector<CampaignPtr>(frontend, cols, rows), _campaignManager(campaignManager)
{
	defaults();
	setColsRowsFromTexture("icon-campaign");
	reset();
}

UINodeCampaignSelector::~UINodeCampaignSelector ()
{
}

void UINodeCampaignSelector::visitCampaign (CampaignPtr& campaign)
{
	addData(campaign);
}

bool UINodeCampaignSelector::onSelect (const CampaignPtr& data)
{
	const CampaignPtr& campaign = _campaignManager.activateCampaign(data->getId());
	if (!campaign)
		return false;

	UI::get().push(UI_WINDOW_CAMPAIGN_MAPS);
	return true;
}

void UINodeCampaignSelector::renderSelectorEntry (int index, const CampaignPtr& data, int x, int y, int colWidth, int rowHeight, float alpha) const
{
	const std::string icon = data->getIcon();
	if (!icon.empty()) {
		TexturePtr t = loadTexture(icon);
		if (t->isValid())
			renderImage(t, x, y, colWidth, rowHeight, alpha);
		else
			renderImage("icon-campaign", x, y, colWidth, rowHeight, alpha);
	} else {
		renderImage("icon-campaign", x, y, colWidth, rowHeight, alpha);
	}

	if (!data->getText().empty()) {
		const BitmapFontPtr& font = getFont(SMALL_FONT);
		const int textHeight = font->getTextHeight(data->getText());
		const int fontX = x + colWidth / 2 - font->getTextWidth(data->getText()) / 2;
		const int fontY = y + rowHeight - textHeight - 1;
		_frontend->renderFilledRect(x, fontY - 1, colWidth, textHeight + 2, highlightColor);
		_frontend->renderRect(x, fontY - 1, colWidth, textHeight + 2, colorWhite);
		font->print(data->getText(), colorWhite, fontX, fontY);
	}
}

void UINodeCampaignSelector::reset ()
{
	UINodeBackgroundSelector<CampaignPtr>::reset();
	_campaignManager.visitCampaigns(this);
}
