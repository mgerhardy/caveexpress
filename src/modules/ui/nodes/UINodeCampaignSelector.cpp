#include "UINodeCampaignSelector.h"
#include "campaign/CampaignManager.h"
#include "ui/UI.h"

UINodeCampaignSelector::UINodeCampaignSelector (IFrontend *frontend, CampaignManager &campaignManager, int cols, int rows) :
		Super(frontend, cols, rows), _campaignManager(campaignManager)
{
	defaults();
	setPadding(0.001);
	onWindowResize();
	reset();
}

UINodeCampaignSelector::~UINodeCampaignSelector ()
{
}

void UINodeCampaignSelector::onWindowResize()
{
	setColsRowsFromTexture("icon-campaign");
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
	const std::string& icon = data->getIcon();
	TexturePtr t = loadTexture(icon);
	if (!t || !t->isValid())
		t = loadTexture("icon-campaign");

	if (isSmallScreen()) {
		if (!data->getText().empty()) {
			const BitmapFontPtr& font = getFont(HUGE_FONT);
			const int textHeight = font->getTextHeight(data->getText());
			const int fontX = std::max(x, x + colWidth / 2 - font->getTextWidth(data->getText()) / 2);
			const int fontY = y + rowHeight - textHeight - 1;
			renderFilledRect(x, fontY - 1, colWidth - _padding, textHeight + 2, colorBlack);
			renderImage(t, x, y, colWidth, rowHeight - textHeight, alpha);
			font->printMax(data->getText(), colorWhite, fontX, fontY, colWidth);
		} else {
			renderImage(t, x, y, colWidth, rowHeight, alpha);
		}
	}else{
		const int marginX = 7 * colWidth / 50, padding = 4 * colWidth / 50, marginXtotal = 2 * marginX + padding;
		if (!data->getText().empty()) {

			const BitmapFontPtr& font = getFont(HUGE_FONT);
			const int textHeight = font->getTextHeight(data->getText());
			const int fontX = std::max(x, x + colWidth / 2 - font->getTextWidth(data->getText()) / 2 - padding / 2 );
			const int fontY = y + rowHeight - textHeight - 1 - padding;

			_frontend->renderFilledRect(x, fontY - 1, colWidth - padding, textHeight + 2, colorBlack);
			renderImage(t, x + marginX, y, colWidth - marginXtotal, rowHeight - textHeight - padding, alpha);
			font->printMax(data->getText(), colorWhite, fontX, fontY, colWidth);
		} else {
			renderImage(t, x + marginX, y, colWidth - marginXtotal, rowHeight - padding, alpha);
		}
	}
}

void UINodeCampaignSelector::reset ()
{
	Super::reset();
	_campaignManager.visitCampaigns(this);
}
