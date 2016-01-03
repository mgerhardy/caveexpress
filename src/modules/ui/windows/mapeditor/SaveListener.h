#pragma once

#include "ui/nodes/UINodeButtonText.h"
#include "common/CommandSystem.h"
#include "common/Log.h"

// TODO: Undo/Redo does not yet work
class UINodeSaveButton: public UINodeButtonText {
public:
	explicit UINodeSaveButton (IFrontend *frontend) :
			UINodeButtonText(frontend, tr("Save & Go"))
	{
		disable();
	}

	void enable () {
		if (_enabled)
			return;
		Log::info(LOG_UI, "Playing the map is now possible");
		setEnabled(true);
		setTooltip("");
	}

	void disable () {
		if (!_enabled)
			return;
		Log::info(LOG_UI, "Playing the map is not possible");
		setEnabled(false);
	}
};

class SaveListener: public UINodeListener, public IMapEditorListener {
private:
	IUINodeMapEditor *_mapEditor;
	UINodeSaveButton *_goButton;
	bool _startMap;
public:
	SaveListener (IUINodeMapEditor *mapEditor, UINodeSaveButton* goButton = nullptr, bool startMap = false) :
			_mapEditor(mapEditor), _goButton(goButton), _startMap(startMap)
	{
		_mapEditor->addEditorListener(this);
	}

	void onClick () override
	{
		if (!_mapEditor->save()) {
			Log::error(LOG_UI, "Failed to save the map");
			return;
		}

		Log::info(LOG_UI, "Saved the map");
		if (_startMap) {
			const std::string& name = _mapEditor->getName();
			if (name.empty()) {
				Log::error(LOG_UI, "Failed to start a map - no name is provided");
				return;
			}
			Log::info(LOG_UI, "Starting the map now: %s", name.c_str());
			Commands.executeCommandLine(CMD_MAP_START " " + name);
		}
	}

	void onSettingsValueChange (const std::string& key, const std::string& value) override
	{
		checkState();
	}

	inline void checkState() {
		enable();
	}

	inline void enable() {
		if (_goButton != nullptr) {
			_goButton->enable();
		}
	}

	inline void disable(const std::string& reason = "") {
		if (_goButton != nullptr) {
			_goButton->disable();
			_goButton->setTooltip(reason);
		}
	}

	void onTilePlaced (const SpriteDefPtr& def) override
	{
		checkState();
	}

	void onNewMap () override
	{
		checkState();
	}

	void onTileRemoved (const SpriteDefPtr& def) override
	{
		checkState();
	}
};
