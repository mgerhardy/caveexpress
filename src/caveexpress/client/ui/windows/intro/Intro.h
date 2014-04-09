#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class Intro: public UIWindow {
public:
	Intro(const std::string& name, IFrontend* frontend);

	virtual ~Intro() {
	}

protected:
	virtual void addIntroNodes() {}
};
