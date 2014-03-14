#pragma once

class EntitySelectionListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeEntitySelector *_selector;
	UINodeMapEditorSelectedItem *_selectedItem;
public:
	EntitySelectionListener (UINodeMapEditor *mapEditor, UINodeEntitySelector *selector, UINodeMapEditorSelectedItem *selectedItem) :
			_mapEditor(mapEditor), _selector(selector), _selectedItem(selectedItem)
	{
	}

	void onClick () override
	{
		const EntityTypeWrapper* p = _selector->getSelection();
		_mapEditor->setEmitterEntity(*p->type);
		const Animation& animation = EntityTypes::hasDirection(*p->type) ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE;
		const SpriteDefPtr& spriteDef = SpriteDefinition::get().getFromEntityType(*p->type, animation);
		_selectedItem->setSprite(spriteDef);
	}
};
