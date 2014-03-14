#pragma once

class AutoGenerateListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeSpriteSelector *_spriteSelector;
public:
	AutoGenerateListener (UINodeMapEditor *mapEditor, UINodeSpriteSelector *spriteSelector) :
			_mapEditor(mapEditor), _spriteSelector(spriteSelector)
	{
	}

	void onClick () override
	{
		_mapEditor->autoFill(_spriteSelector->getTheme());
	}
};
