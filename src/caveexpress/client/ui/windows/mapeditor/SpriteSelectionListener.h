#pragma once

namespace caveexpress {

class SpriteSelectionListener: public UINodeListener, IMapEditorListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeSpriteSelector *_selector;
	UINodeMapEditorSelectedItem *_selectedItem;
public:
	SpriteSelectionListener (UINodeMapEditor *mapEditor, UINodeSpriteSelector *selector, UINodeMapEditorSelectedItem *selectedItem) :
			_mapEditor(mapEditor), _selector(selector), _selectedItem(selectedItem)
	{
		_mapEditor->addEditorListener(this);
	}

	void onClick () override
	{
		SpriteDefPtr *p = _selector->getSelection();
		if (!p)
			return;
		_mapEditor->setSprite(*p);
		_selectedItem->setSprite(*p);
	}

	void onTileSelected (const SpriteDefPtr& def) override
	{
		_selectedItem->setSprite(def);
	}
};

}
