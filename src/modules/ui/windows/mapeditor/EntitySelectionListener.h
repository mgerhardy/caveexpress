#pragma once

class EntitySelectionListener: public UINodeListener {
private:
	IUINodeMapEditor *_mapEditor;
	IUINodeEntitySelector *_selector;
	UINodeMapEditorSelectedItem *_selectedItem;
public:
	EntitySelectionListener (IUINodeMapEditor *mapEditor, IUINodeEntitySelector *selector, UINodeMapEditorSelectedItem *selectedItem) :
			_mapEditor(mapEditor), _selector(selector), _selectedItem(selectedItem)
	{
	}

	void onClick () override
	{
		const EntityTypeWrapper* p = _selector->getSelection();
		_mapEditor->setEmitterEntity(*p->type);
		const Animation& animation = Animation::NONE;
		const SpriteDefPtr& spriteDef = SpriteDefinition::get().getFromEntityType(*p->type, animation);
		_selectedItem->setSprite(spriteDef);
	}
};
