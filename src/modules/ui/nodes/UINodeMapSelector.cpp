#include "UINodeMapSelector.h"
#include "common/MapManager.h"
#include "common/CommandSystem.h"
#include "campaign/CampaignManager.h"
#include "common/Commands.h"

UINodeMapSelector::UINodeMapSelector (IFrontend *frontend, const IMapManager &mapManager, bool multiplayer, int cols, int rows) :
		UINodeBackgroundSelector<std::string>(frontend, cols, rows), _campaignManager(nullptr), _mapManager(&mapManager), _multiplayer(multiplayer)
{
	setColsRowsFromTexture("map-icon-locked");
	defaults();
	setPaddingPixel(10);
	reset();
}

UINodeMapSelector::UINodeMapSelector (IFrontend *frontend, CampaignManager &campaignManager, bool multiplayer, int cols, int rows) :
		UINodeBackgroundSelector<std::string>(frontend, cols, rows), _campaignManager(&campaignManager), _mapManager(
				nullptr), _multiplayer(multiplayer)
{
	setColsRowsFromTexture("map-icon-locked");
	defaults();
	setPaddingPixel(10);
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

	const TexturePtr t = getIcon(data);
	if (_campaignManager != nullptr) {
		const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
		const CampaignMap *map = campaignPtr->getMapById(data);
		if (map != nullptr && !map->isLocked()) {
			const BitmapFontPtr& font = getFont(MEDIUM_FONT);
			const std::string points = string::toString(map->getFinishPoints());
			const int fontX = std::max(x, x + colWidth / 2 - font->getTextWidth(points) / 2);
			const int fontHeight = font->getTextHeight(points);
			const int fontY = y + fontHeight;
			if (t)
				renderImage(t, x, y, colWidth, rowHeight - fontHeight, alpha);
			font->printMax(points, colorWhite, fontX, fontY, colWidth);
		} else if (t) {
			renderImage(t, x, y, colWidth, rowHeight, alpha);
		}
	} else if (t) {
		renderImage(t, x, y, colWidth, rowHeight, alpha);
	}

/*	if (_selectedIndex != index)
		return;*/

	if (!title.empty()) {
		const BitmapFontPtr& font = getFont(SMALL_FONT);
		const int textHeight = font->getTextHeight(title);
		const int fontX = std::max(x, x + colWidth / 2 - font->getTextWidth(title) / 2);
		const int fontY = y + rowHeight - textHeight - 1;
		_frontend->renderFilledRect(x, fontY - 1, colWidth, textHeight + 2, colorBlack);
		//_frontend->renderRect(x, fontY - 1, colWidth, textHeight + 2, colorWhite);
		font->printMax(title, colorWhite, fontX, fontY, colWidth);
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
			if (!_multiplayer || i->second->getStartPositions() > 1)
				addData(i->first);
		}
		return;
	}

	const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
	if (!campaignPtr)
		return;
	const Campaign::MapList& maps = campaignPtr->getMaps();
	int index = -1;
	int mapIndex = 0;
	for (Campaign::MapListConstIter i = maps.begin(); i != maps.end(); ++i, ++mapIndex) {
		const CampaignMapPtr& p = *i;
		addData(p->getId());
		if (index == -1 && p->isLocked()) {
			index = mapIndex - 1;
		}
	}
	selectEntry(index);
}
