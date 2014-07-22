#include "UINodeMapSelector.h"
#include "engine/common/MapManager.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/campaign/CampaignManager.h"
#include "engine/common/Commands.h"
#include "engine/common/FileSystem.h"

UINodeMapSelector::UINodeMapSelector (IFrontend *frontend, const IMapManager &mapManager, int cols, int rows) :
		UINodeBackgroundSelector<std::string>(frontend, cols, rows), _campaignManager(nullptr), _mapManager(&mapManager)
{
	setColsRowsFromTexture("map-icon-locked");
	defaults();
	reset();
}

UINodeMapSelector::UINodeMapSelector (IFrontend *frontend, CampaignManager &campaignManager, int cols, int rows) :
		UINodeBackgroundSelector<std::string>(frontend, cols, rows), _campaignManager(&campaignManager), _mapManager(
				nullptr)
{
	setColsRowsFromTexture("map-icon-locked");
	defaults();
	reset();
}

UINodeMapSelector::~UINodeMapSelector ()
{
}

bool UINodeMapSelector::onSelect (const std::string& data)
{
	if (_campaignManager) {
		const CampaignPtr& campaign = _campaignManager->getActiveCampaign();
		if (campaign) {
			const CampaignMap* map = campaign->getMapById(data);
			if (map != nullptr) {
				if (map->isLocked())
					return false;
				_campaignManager->startMap(data);
				return true;
			}
		}
	}

	Commands.executeCommandLine(CMD_MAP_START " " + data);
	return true;
}

void UINodeMapSelector::renderSelectorEntry (int index, const std::string& data, int x, int y, int colWidth,
		int rowHeight, float alpha) const
{
	const TexturePtr t = getIcon(data);
	if (t)
		renderImage(t, x, y, colWidth, rowHeight, alpha);

	if (_campaignManager != nullptr) {
		const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
		const CampaignMap *map = campaignPtr->getMapById(data);
		if (map != nullptr && !map->isLocked()) {
			const BitmapFontPtr& font = getFont(MEDIUM_FONT);
			const std::string points = string::toString(map->getFinishPoints());
			const int fontX = x + colWidth / 2 - font->getTextWidth(points) / 2;
			const int fontY = y + font->getTextHeight(points);
			font->print(points, colorWhite, fontX, fontY);
		}
	}

	if (_selectedIndex != index)
		return;

	std::string title;
	if (_mapManager != nullptr) {
		title = _mapManager->getMapTitle(data);
	} else {
		const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
		const CampaignMap *map = campaignPtr->getMapById(data);
		if (map == nullptr)
			return;
		title = map->getName();
	}

	if (!title.empty()) {
		const BitmapFontPtr& font = getFont(SMALL_FONT);
		const int textHeight = font->getTextHeight(title);
		const int fontX = x + colWidth / 2 - font->getTextWidth(title) / 2;
		const int fontY = y + rowHeight - textHeight - 1;
		_frontend->renderFilledRect(x, fontY - 1, colWidth, textHeight + 2, highlightColor);
		_frontend->renderRect(x, fontY - 1, colWidth, textHeight + 2, colorWhite);
		font->print(title, colorWhite, fontX, fontY);
	}
}

int UINodeMapSelector::getLives () const
{
	if (_campaignManager == nullptr)
		return 0;
	const CampaignPtr& c = _campaignManager->getActiveCampaign();
	return c->getLives();
}

TexturePtr UINodeMapSelector::getIcon (const std::string& data) const
{
	if (_campaignManager != nullptr) {
		const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
		const CampaignMap* map = campaignPtr->getMapById(data);
		if (map != nullptr) {
			if (map->isLocked())
				return loadTexture("map-icon-locked");

			const TexturePtr& ptr = loadTexture("map-icon-unlocked-" + string::toString(static_cast<int>(map->getStars())));
			if (ptr)
				return ptr;
		}
	}
	return loadTexture("map-icon-unlocked");
}

void UINodeMapSelector::reset ()
{
	UINodeSelector<std::string>::reset();
	if (_mapManager) {
		const IMapManager::Maps &maps = _mapManager->getMaps();
		for (IMapManager::Maps::const_iterator i = maps.begin(); i != maps.end(); ++i) {
			addData(i->first);
		}
		return;
	}

	const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
	if (!campaignPtr)
		return;
	const Campaign::MapList& maps = campaignPtr->getMaps();
	for (Campaign::MapListConstIter i = maps.begin(); i != maps.end(); ++i) {
		addData((*i)->getId());
	}
}
