#pragma once
#include "common/ConfigManager.h"

class FullscreenListener: public UINodeListener {
private:
	IFrontend *_frontend;
	bool _on;
public:
	FullscreenListener (IFrontend *frontend, bool on) :
		_frontend(frontend), _on(on)
	{
	}

	void onClick () override
	{
		_frontend->setFullscreen(_on);
	}
};
