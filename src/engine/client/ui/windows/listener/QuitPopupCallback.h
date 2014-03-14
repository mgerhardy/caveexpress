#pragma once

#include "engine/common/Commands.h"
#include "engine/common/CommandSystem.h"
#include "engine/client/ui/UI.h"

class QuitPopupCallback: public UIPopupCallback {
public:
	void onOk() override {
		Commands.executeCommand(CMD_QUIT);
	}

	void onCancel() override {
		UI::get().delayedPop();
	}
};
