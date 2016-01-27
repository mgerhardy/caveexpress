#pragma once

#include "common/ConfigManager.h"
#include "ui/nodes/UINodeSlider.h"
#include "ui/nodes/UINodeTextInput.h"

class ConfigVarListener: public UINodeListener {
protected:
	const std::string _configVarName;
	const UINodeTextInput* _textInput;
	const UINodeSlider* _slider;

public:
	ConfigVarListener (const std::string& configVarName, const UINodeTextInput* textInput) :
			_configVarName(configVarName), _textInput(textInput), _slider(nullptr)
	{
	}

	ConfigVarListener (const std::string& configVarName, const UINodeSlider* slider) :
			_configVarName(configVarName), _textInput(nullptr), _slider(slider)
	{
	}

	void onValueChanged () override
	{
		ConfigVarPtr var = Config.getConfigVar(_configVarName);
		if (_textInput != nullptr)
			var->setValue(_textInput->getValue());
		else if (_slider != nullptr)
			var->setValue(_slider->getValue());
	}
};
