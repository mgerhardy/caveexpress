#pragma once

#include "ui/nodes/UINode.h"
#include "common/SpriteDefinition.h"

class UINodeMapEditorSelectedItem: public UINode {
private:
	SpriteDefPtr _activeSpriteDefition;

public:
	UINodeMapEditorSelectedItem (IFrontend *frontend);
	virtual ~UINodeMapEditorSelectedItem ();
	void setSprite (const SpriteDefPtr& spriteDef);

	void render (int x, int y) const override;
};

inline void UINodeMapEditorSelectedItem::setSprite (const SpriteDefPtr& spriteDef)
{
	_activeSpriteDefition = spriteDef;
}
