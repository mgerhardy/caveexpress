#pragma once

#include "engine/client/ui/UI.h"
#include "engine/client/ui/windows/listener/QuitPopupCallback.h"

class QuitListener: public UINodeListener {
public:
	void onClick() override {
		UIPopupCallbackPtr c(new QuitPopupCallback());
		UI::get().popup(tr("Quit"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
	}
};
