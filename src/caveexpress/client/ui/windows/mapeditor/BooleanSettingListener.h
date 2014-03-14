#pragma once

class BooleanSettingListener: public UINodeListener, IMapEditorListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeCheckbox *_checkboxNode;
	bool _blocked;
	const std::string _key;
public:
	BooleanSettingListener (UINodeMapEditor *mapEditor, UINodeCheckbox *checkboxNode, const std::string& key) :
			_mapEditor(mapEditor), _checkboxNode(checkboxNode), _blocked(false), _key(key)
	{
		_mapEditor->addEditorListener(this);
	}

	void onSettingsValueChange (const std::string& key, const std::string& value) override
	{
		if (_blocked)
			return;

		if (key == _key) {
			const bool selected = string::toBool(value);
			_checkboxNode->setSelected(selected);
		}
	}

	void onValueChanged () override
	{
		_blocked = true;
		const std::string value = _checkboxNode->isSelected() ? "true" : "false";
		_mapEditor->setSetting(_key, value);
		_blocked = false;
	}
};
