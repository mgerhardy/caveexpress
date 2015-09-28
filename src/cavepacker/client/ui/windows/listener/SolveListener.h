#pragma once

#include "ui/nodes/UINode.h"
#include "ui/nodes/UINodeSlider.h"
#include "common/ConfigManager.h"
#include <string>

class SolveListener: public UINodeListener {
private:
	UINodeSlider *_sliderNode;
	std::string _configVar;
public:
	SolveListener (UINodeSlider *sliderNode, const std::string& configVar) :
			_sliderNode(sliderNode), _configVar(configVar)
	{
		_sliderNode->setValue(Config.getConfigVar(_configVar)->getFloatValue());
	}

	void onValueChanged () override
	{
		const float val = _sliderNode->getValue();
		Config.getConfigVar(_configVar)->setValue(string::toString(val));
	}
};
