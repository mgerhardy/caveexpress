#pragma once

#include "ui/windows/UIWindow.h"
#include "ui/nodes/UINodeSelector.h"

class IMapManager;
class UINodeMapStringSelector;

namespace caveexpress {

// forward decl
class UINodeMapEditorSelectedItem;
class UINodeSpriteSelector;
class UINodeEntitySelector;
class UINodeMapEditor;

class UIMapEditorWindow: public UIWindow {
	friend class AutoGenerateListener;
private:
	UINodeSpriteSelector *_spritesNode;
	UINodeEntitySelector *_emitterNode;
	UINodeMapEditorSelectedItem *_selectedItemNode;
	UINodeMapEditor *_mapEditor;

	std::string _theme;

	UINode *createSettings ();
	UINode *createLayers ();
	UINode *createButtons (IMapManager& mapManager, UINodeMapStringSelector *mapListNode);
public:
	UIMapEditorWindow (IFrontend *frontend, IMapManager& mapManager);
	virtual ~UIMapEditorWindow ();

	bool nextFocus () override
	{
		return false;
	}
	bool prevFocus () override
	{
		return false;
	}

	inline UINodeMapEditor *getMapEditorNode ()
	{
		return _mapEditor;
	}

	void onSelectEntry (UINode &node);

	bool onPush () override;
};

}
