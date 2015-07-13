#pragma once

#include "ui/nodes/IUINodeSpriteSelector.h"
#include "ui/nodes/IUINodeEntitySelector.h"
#include "ui/nodes/IUINodeMapEditor.h"
#include "ui/nodes/UINodeMapEditorSelectedItem.h"

class ChangeThemeListener: public UINodeListener, IMapEditorListener {
private:
	IUINodeMapEditor *_mapEditor;
	IUINodeSpriteSelector *_spriteSelector;
	IUINodeEntitySelector* _entitySelector;
	UINodeMapEditorSelectedItem *_selectedItem;
	const ThemeType& _theme;
	bool _blocked;
public:
	ChangeThemeListener (IUINodeMapEditor *mapEditor, IUINodeSpriteSelector *spriteSelector,
			IUINodeEntitySelector* entitySelector, UINodeMapEditorSelectedItem *selectedItem, const ThemeType& theme) :
			_mapEditor(mapEditor), _spriteSelector(spriteSelector), _entitySelector(entitySelector), _selectedItem(
					selectedItem), _theme(theme), _blocked(false)
	{
		_mapEditor->addEditorListener(this);
	}

	void onSettingsValueChange (const std::string& key, const std::string& value) override
	{
		if (_blocked)
			return;

		if (key != msn::THEME)
			return;

		const ThemeType& theme = ThemeType::getByName(value);
		if (theme != _theme)
			return;

		onClick();
	}

	void onClick () override
	{
		_blocked = true;
		_spriteSelector->addSprites(_theme);
		_entitySelector->addEntities(_theme);
		_mapEditor->setTheme(_theme);
		const SpriteDefPtr* spriteDef = _spriteSelector->getSelection();
		if (spriteDef != nullptr) {
			_mapEditor->setSprite(*spriteDef);
			_selectedItem->setSprite(*spriteDef);
		} else {
			_mapEditor->setSprite(SpriteDefPtr());
			_selectedItem->setSprite(SpriteDefPtr());
		}
		_blocked = false;
	}
};
