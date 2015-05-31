#pragma once

#include "ui/nodes/UINode.h"

class OpenWindowListener: public UINodeListener {
	const std::string _window;
public:
	OpenWindowListener(const std::string& window) :
			_window(window) {
	}

	void onClick() override {
		UI::get().push(_window);
	}
};
