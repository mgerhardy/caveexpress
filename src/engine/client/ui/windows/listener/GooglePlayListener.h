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
			UIPopupCallbackPtr c(new UIPopupOkCommandCallback("googleplay-disconnect"));
			UI::get().popup(tr("Google Play sign-out"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
		} else {
			Commands.executeCommandLine("googleplay-connect");
		}
	}
};

