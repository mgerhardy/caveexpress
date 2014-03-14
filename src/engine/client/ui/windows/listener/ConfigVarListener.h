#pragma once

#include "engine/common/ConfigManager.h"

class ConfigVarListener: public UINodeListener {
protected:
	const std::string _configVarName;
	const UINodeTextInput* _textInput;

public:
	ConfigVarListener (const std::string& configVarName, const UINodeTextInput* textInput) :
			_configVarName(configVarName), _textInput(textInput)
	{
	}

	void onValueChanged () override
	{
		ConfigVarPtr var = Config.getConfigVar(_configVarName);
		var->setValue(_textInput->getValue());
	}
};
