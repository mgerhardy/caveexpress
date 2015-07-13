#pragma once

#include "ui/nodes/UINodeSelector.h"
#include "common/SpriteDefinition.h"

class IUINodeSpriteSelector: public UINodeSelector<SpriteDefPtr> {
private:
	const ThemeType* _theme;
protected:
	// can be used to filter out some sprites
	virtual bool shouldBeShown(const SpriteDefPtr& sprite) const = 0;
public:
	IUINodeSpriteSelector (IFrontend *frontend, int cols = -1, int rows = -1);
	virtual ~IUINodeSpriteSelector ();

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
