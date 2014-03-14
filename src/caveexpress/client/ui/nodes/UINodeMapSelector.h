#pragma once

#include "engine/client/ui/nodes/UINodeSelector.h"

#include <vector>
#include <string>

// forward decl
class MapManager;
class CampaignManager;

class UINodeMapSelector: public UINodeBackgroundSelector<std::string> {
private:
	CampaignManager *_campaignManager;
	const MapManager *_mapManager;
public:
	UINodeMapSelector (IFrontend *frontend, const MapManager &mapManager, int cols = 6, int rows = 4);
	UINodeMapSelector (IFrontend *frontend, CampaignManager &campaignManager, int cols = 6, int rows = 4);
	virtual ~UINodeMapSelector ();

	int getLives () const;

	// UINodeSelector
	bool onSelect (const std::string& data) override;
	TexturePtr getIcon (const std::string& data) const override;
	void renderSelectorEntry (int index, const std::string& data, int x, int y, int colWidth, int rowHeight,
			float alpha) const override;
	void reset () override;
};
