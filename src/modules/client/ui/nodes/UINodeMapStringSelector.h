#pragma once

#include "client/ui/nodes/UINodeSelector.h"

#include <vector>
#include <string>

// forward decl
class IMapManager;

class UINodeMapStringSelector: public UINodeSelector<std::string> {
private:
	const IMapManager &_mapManager;
public:
	UINodeMapStringSelector (IFrontend *frontend, const IMapManager &mapManager, int rows);
	virtual ~UINodeMapStringSelector ();

	// UINode
	float getAutoWidth () const override;
	float getAutoHeight () const override;

	// UINodeSelector
	std::string getText (const std::string& data) const override;
	void renderSelectorEntry (int index, const std::string& data, int x, int y, int colWidth, int rowHeight, float alpha) const override;
	void reset () override;
};
