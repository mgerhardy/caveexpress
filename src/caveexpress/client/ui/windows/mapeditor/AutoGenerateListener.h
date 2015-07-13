#pragma once

#include "caveexpress/client/ui/nodes/UINodeSpriteSelector.h"
#include "caveexpress/client/ui/nodes/UINodeMapEditor.h"

namespace caveexpress {

class AutoGenerateListener: public UINodeListener {
private:
	IUINodeMapEditor *_mapEditor;
	IUINodeSpriteSelector *_spriteSelector;
public:
	AutoGenerateListener (IUINodeMapEditor *mapEditor, IUINodeSpriteSelector *spriteSelector) :
			_mapEditor(mapEditor), _spriteSelector(spriteSelector)
	{
	}

	void onClick () override
	{
		((UINodeMapEditor*)_mapEditor)->autoFill(_spriteSelector->getTheme());
	}
};

}
