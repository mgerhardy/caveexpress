#pragma once

namespace caveexpress {

class IntSettingsListener: public UINodeListener, IMapEditorListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeSpinner *_spinnerNode;
	bool _blocked;
	const std::string _key;
public:
	IntSettingsListener (UINodeMapEditor *mapEditor, UINodeSpinner *spinnerNode, const std::string& key) :
			_mapEditor(mapEditor), _spinnerNode(spinnerNode), _blocked(false), _key(key)
	{
		_mapEditor->addEditorListener(this);
	}

	void onSettingsValueChange (const std::string& key, const std::string& value) override
	{
		if (_blocked)
			return;

		if (key != _key)
			return;

		const int val = string::toInt(value);
		_spinnerNode->setValue(val);
	}

	void onValueChanged () override
	{
		_blocked = true;
		const int val = _spinnerNode->getValue();
		_mapEditor->setSetting(_key, string::toString(val));
		_blocked = false;
	}
};

}
