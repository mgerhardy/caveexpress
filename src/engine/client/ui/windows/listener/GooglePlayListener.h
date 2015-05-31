#pragma once

#include "engine/common/CommandSystem.h"
#include "engine/common/ConfigManager.h"
#include "engine/client/ui/UI.h"

class GooglePlayListener: public UINodeListener {
private:
	ConfigVarPtr _googlePlay;
public:
	GooglePlayListener() {
		_googlePlay = Config.getConfigVar("googleplaystate");
	}

	void onClick() override
	{
		if (_googlePlay->getBoolValue()) {
			UI::get().push(UI_WINDOW_GOOGLEPLAY);
		} else {
			Commands.executeCommandLine("googleplay-connect");
		}
	}
};

