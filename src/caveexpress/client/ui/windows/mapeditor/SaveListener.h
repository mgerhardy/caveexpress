#pragma once

#include "engine/client/ui/nodes/UINodeButtonText.h"
#include "engine/common/CommandSystem.h"

// TODO: Undo/Redo does not yet work
class UINodeSaveButton: public UINodeButtonText {
public:
	UINodeSaveButton (IFrontend *frontend) :
			UINodeButtonText(frontend, tr("Save & Go"))
	{
		disable();
	}

	void enable () {
		info(LOG_CLIENT, "Playing the map is now possible");
		setEnabled(true);
		setTooltip("");
	}

	void disable () {
		info(LOG_CLIENT, "Playing the map is not possible");
		setEnabled(false);
		setTooltip(tr("Can't start the map without a packagetarget"));
	}
};

class SaveListener: public UINodeListener, public IMapEditorListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeSaveButton *_goButton;
	bool _startMap;
	int _packageTargets;
public:
	SaveListener (UINodeMapEditor *mapEditor, UINodeSaveButton* goButton = nullptr, bool startMap = false) :
			_mapEditor(mapEditor), _goButton(goButton), _startMap(startMap), _packageTargets(0)
	{
		_mapEditor->addEditorListener(this);
	}

	void onClick () override
	{
		_mapEditor->save();
		if (_startMap)
			Commands.executeCommandLine(CMD_MAP_START " " + _mapEditor->getName());
	}

	void onTilePlaced (const SpriteDefPtr& def) override
	{
		if (!SpriteTypes::isPackageTarget(def->type))
			return;
		++_packageTargets;
		if (_goButton != nullptr) {
			_goButton->enable();
		}
	}

	void onNewMap () override
	{
		_packageTargets = 0;
		if (_goButton != nullptr) {
			_goButton->disable();
		}
	}

	void onTileRemoved (const SpriteDefPtr& def) override
	{
		if (!SpriteTypes::isPackageTarget(def->type))
			return;
		--_packageTargets;
		if (_packageTargets <= 0 && _goButton != nullptr) {
			_goButton->disable();
		}
	}
};
