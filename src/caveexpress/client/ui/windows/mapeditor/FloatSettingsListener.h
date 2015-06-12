#pragma once

namespace caveexpress {

class FloatSettingsListener: public UINodeListener, IMapEditorListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeSlider *_sliderNode;
	bool _blocked;
	const std::string _key;
public:
	FloatSettingsListener (UINodeMapEditor *mapEditor, UINodeSlider *sliderNode, const std::string& key) :
			_mapEditor(mapEditor), _sliderNode(sliderNode), _blocked(false), _key(key)
	{
		_mapEditor->addEditorListener(this);
	}

	void onSettingsValueChange (const std::string& key, const std::string& value) override
	{
		if (_blocked)
			return;

		if (key != _key)
			return;

		const float val = string::toFloat(value);
		_sliderNode->setValue(val);
	}

	void onValueChanged () override
	{
		_blocked = true;
		const float val = _sliderNode->getValue();
		_mapEditor->setSetting(_key, string::toString(val));
		_blocked = false;
	}
};

}
