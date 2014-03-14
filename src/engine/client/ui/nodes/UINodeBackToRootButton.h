#pragma once

#include "engine/client/ui/nodes/UINodeButtonImage.h"
#include "engine/client/ui/UI.h"
#include "engine/common/CommandSystem.h"

class UINodeBackToRootButton: public UINodeButtonImage {
protected:
	bool execute () override
	{
		Commands.executeCommandLine(CMD_CL_DISCONNECT);
		UI::get().popMain();
		return UINodeButton::execute();
	}
public:
	UINodeBackToRootButton (IFrontend *frontend) :
		UINodeButtonImage(frontend, "icon-back-to-root")
	{
		setId("back-to-root");
	}
};
