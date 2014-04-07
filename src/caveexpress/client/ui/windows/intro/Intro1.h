#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class Intro1: public UIWindow {
public:
	Intro1 (IFrontend* frontend);
	virtual ~Intro1 ();

	void onActive () override;
	bool onPop () override;
};
