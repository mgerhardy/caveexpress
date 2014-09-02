#pragma once

#include "engine/client/ui/UI.h"
#include "engine/client/ui/windows/UIWindow.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/String.h"
#include <string>
#include <vector>

class SetDifficultyNodeListener: public UINodeListener {
private:
	UINodeButton* _button;
	std::vector<UINodeButton*> _buttons;
public:
	SetDifficultyNodeListener (UINodeButton* button, std::vector<UINodeButton*>& buttons) :
			_button(button), _buttons(buttons)
	{
	}

	void onClick () override
	{
		for (std::vector<UINodeButton*>::iterator i = _buttons.begin(); i != _buttons.end(); ++i) {
			if (*i == _button) {
				_button->setBackgroundColor(colorBrightGreen);
				_button->setImage("");
				Config.getConfigVar("difficulty")->setValue(string::toLower(_button->getTitle()));
			} else {
				(*i)->setBackgroundColor(colorNull);
				(*i)->setImage("bg");
			}
		}
	}
};
