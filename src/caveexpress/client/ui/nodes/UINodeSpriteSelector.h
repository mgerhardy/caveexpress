#pragma once

#include "client/ui/nodes/UINodeSelector.h"
#include "common/SpriteDefinition.h"

class UINodeSpriteSelector: public UINodeSelector<SpriteDefPtr> {
private:
	const ThemeType* _theme;
public:
	UINodeSpriteSelector (IFrontend *frontend, int cols, int rows);
	virtual ~UINodeSpriteSelector ();

	void addSprites (const ThemeType& theme);

	inline const ThemeType& getTheme () const
	{
		return *_theme;
	}

	// UINode
	void update (uint32_t deltaTime) override;

	// UINodeSelector
	void renderSelectorEntry (int index, const SpriteDefPtr& data, int x, int y, int colWidth, int rowHeight, float alpha) const override;
};
