#pragma once

#include "common/ConfigManager.h"
#include "ui/nodes/UINodeSlider.h"
#include "ui/nodes/UINodeTextInput.h"
#include "ui/nodes/UINodeSpinner.h"

class ConfigVarListener: public UINodeListener {
protected:
	const std::string _configVarName;
	const UINodeTextInput* _textInput;
	const UINodeSlider* _slider;
	const UINodeSpinner* _spinner;

public:
	ConfigVarListener (const std::string& configVarName, const UINodeTextInput* textInput) :
			_configVarName(configVarName), _textInput(textInput), _slider(nullptr), _spinner(nullptr)
	{
	}

	ConfigVarListener (const std::string& configVarName, const UINodeSlider* slider) :
			_configVarName(configVarName), _textInput(nullptr), _slider(slider), _spinner(nullptr)
	{
	}

	ConfigVarListener (const std::string& configVarName, const UINodeSpinner* spinner) :
			_configVarName(configVarName), _textInput(nullptr), _slider(nullptr), _spinner(spinner)
	{
	}

	void onValueChanged () override
	{
		ConfigVarPtr var = Config.getConfigVar(_configVarName);
		if (_textInput != nullptr)
			var->setValue(_textInput->getValue());
		else if (_slider != nullptr)
			var->setValue(_slider->getValue());
		else if (_spinner != nullptr)
			var->setValue(_spinner->getValue());
	}
};
