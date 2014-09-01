#pragma once

#include "UINodeButton.h"
#include "shared/constants/Commands.h"

class UINodeBackButton: public UINodeButton {
public:
	UINodeBackButton (IFrontend *frontend) :
			UINodeButton(frontend)
	{
		setTitle("back");
		setOnActivate(CMD_UI_POP);
	}

	virtual ~UINodeBackButton ()
	{
	}
};
