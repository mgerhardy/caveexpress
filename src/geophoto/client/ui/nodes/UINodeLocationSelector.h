#pragma once

#include "UINodeSelector.h"
#include "client/round/GameRound.h"

class UINodeLocationSelector: public UINodeSelector<Location> {
private:
	const GameRound& _gameRound;
public:
	UINodeLocationSelector(IFrontend *frontend, const GameRound& gameRound);
	virtual ~UINodeLocationSelector();

	// UINodeSelector
	bool onSelect (const Location& data) override;
	void renderSelectorEntry (int index, const Location& data, int x, int y, int colWidth, int rowHeight, float alpha) const override;
	void reset () override;
};
