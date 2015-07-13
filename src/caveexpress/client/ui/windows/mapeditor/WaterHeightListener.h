#pragma once

#include "caveexpress/client/ui/nodes/UINodeMapEditor.h"
#include "ui/nodes/UINodeSlider.h"

namespace caveexpress {

class WaterHeightListener: public UINodeListener, IMapEditorListener {
private:
	IUINodeMapEditor *_mapEditor;
	UINodeSlider *_sliderNode;
	bool _blocked;
public:
	WaterHeightListener (IUINodeMapEditor *mapEditor, UINodeSlider *sliderNode) :
			_mapEditor(mapEditor), _sliderNode(sliderNode), _blocked(false)
	{
		_mapEditor->addEditorListener(this);
	}

	void onSettingsValueChange (const std::string& key, const std::string& value) override
	{
		if (_blocked)
			return;
		if (key == msn::WATER_HEIGHT) {
			const float waterHeight = string::toFloat(value);
			_sliderNode->setValue(waterHeight);
		} else if (key == msn::HEIGHT) {
			_sliderNode->setMax(string::toFloat(value));
		}
	}

	void onValueChanged () override
	{
		_blocked = true;
		((UINodeMapEditor*)_mapEditor)->setWaterHeight(_sliderNode->getValue());
		_blocked = false;
	}
};

}
