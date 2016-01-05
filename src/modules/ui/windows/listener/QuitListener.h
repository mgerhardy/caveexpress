#pragma once

#include "ui/UI.h"
#include "common/Commands.h"

class QuitListener: public UINodeListener {
public:
	void onClick() override {
		UIPopupCallbackPtr c(new UIPopupOkCommandCallback(CMD_QUIT));
		UI::get().popup(tr("Quit?"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
	}
};
