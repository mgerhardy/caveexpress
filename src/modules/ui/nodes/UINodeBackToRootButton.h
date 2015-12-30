#pragma once

#include "ui/nodes/UINodeMainButton.h"
#include "ui/UI.h"
#include "common/CommandSystem.h"

class UINodeBackToRootButton: public UINodeMainButton {
protected:
	bool execute () override
	{
		Commands.executeCommandLine(CMD_CL_DISCONNECT);
		UI::get().popMain();
		return UINodeButton::execute();
	}
public:
	UINodeBackToRootButton (IFrontend *frontend) :
		UINodeMainButton(frontend, tr("Close"))
	{
		setId("back-to-root");
	}
};
