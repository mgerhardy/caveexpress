#pragma once

#include "ui/windows/UIWindow.h"
#include "ui/nodes/UINodeSelector.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/layouts/UIVBoxLayout.h"

// forward decl
class IMapManager;
class UINodeMapStringSelector;
class UINodeMapEditorSelectedItem;
class IUINodeSpriteSelector;
class IUINodeEntitySelector;
class IUINodeMapEditor;

class UINodeSetting: public UINode {
public:
	UINodeSetting (IFrontend* frontend, const std::string& label, UINode* node, const std::string& tooltip = "") :
			UINode(frontend, label) {
		setTooltip(tooltip);
		setLayout(new UIVBoxLayout(0.01f, true));
		UINodeLabel *labelNode = new UINodeLabel(_frontend, label);
		add(labelNode);
		add(node);
	}
};

class IUIMapEditorWindow: public UIWindow {
protected:
	IUINodeSpriteSelector *_spritesNode;
	IUINodeEntitySelector *_emitterNode;
	UINodeMapEditorSelectedItem *_selectedItemNode;
	IUINodeMapEditor *_mapEditor;

	std::string _theme;

	virtual UINode *createSettings ();
	virtual UINode *createLayers ();
	virtual UINode *createButtons (IMapManager& mapManager, UINodeMapStringSelector *mapListNode);
public:
	IUIMapEditorWindow (IFrontend *frontend, IMapManager& mapManager, IUINodeMapEditor* editor, IUINodeSpriteSelector* spriteSelector, IUINodeEntitySelector* entitySelector);
	virtual ~IUIMapEditorWindow ();

	bool nextFocus () override
	{
		return false;
	}
	bool prevFocus () override
	{
		return false;
	}

	inline IUINodeMapEditor *getMapEditorNode ()
	{
		return _mapEditor;
	}

	void onSelectEntry (UINode &node);

	bool onPush () override;
};
