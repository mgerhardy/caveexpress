#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class Intro: public UIWindow {
public:
	Intro(const std::string& name, IFrontend* frontend);

	virtual ~Intro() {
	}

	void init ();

protected:
	UINode *_background;
	UINode *_child;

	virtual void addIntroNodes(UINode* parent) = 0;
};
