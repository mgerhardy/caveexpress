#pragma once

#include "engine/client/ui/UI.h"

class GooglePlayDisconnectListener: public UINodeListener {
public:
	void onClick() override
	{
		UIPopupCallbackPtr c(new UIPopupOkCommandCallback("googleplay-disconnect"));
		UI::get().popup(tr("Google Play sign-out"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
	}
};

