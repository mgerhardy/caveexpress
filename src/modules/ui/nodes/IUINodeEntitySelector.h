#pragma once

#include "common/EntityType.h"
#include "ui/nodes/UINodeSelector.h"
#include "common/ThemeType.h"
#include "sprites/Sprite.h"
#include "common/SpriteDefinition.h"

struct EntityTypeWrapper {
	const EntityType* type;
	SpritePtr sprite;
	EntityAngle angle;
};

class IUINodeEntitySelector: public UINodeSelector<EntityTypeWrapper> {
private:
	const ThemeType* _theme;

	void addEntity (const EntityType& type, const ThemeType& theme);

protected:
	// can be used to filter out some sprites
	virtual bool shouldBeShown(const EntityType& type) const = 0;
	virtual const Animation& getAnimation(const EntityType& type) const = 0;

public:
	IUINodeEntitySelector (IFrontend *frontend, int cols = -1, int rows = -1);
	virtual ~IUINodeEntitySelector ();

	void addEntities (const ThemeType& theme);

	inline const ThemeType& getTheme () const
	{
		return *_theme;
	}

	// UINode
	void update (uint32_t deltaTime) override;

	// UINodeSelector
	void renderSelectorEntry (int index, const EntityTypeWrapper& data, int x, int y, int colWidth, int rowHeight, float alpha) const override;
};
