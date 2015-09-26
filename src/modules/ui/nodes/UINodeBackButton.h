#pragma once

#include "ui/nodes/UINodeMainButton.h"
#include "ui/UI.h"
#include "common/Commands.h"

class UINodeBackButton: public UINodeMainButton {
public:
	UINodeBackButton (IFrontend *frontend, UINode* alignToNode = nullptr) :
		UINodeMainButton(frontend, tr("Back"))
	{
		setId("back-button");
		setOnActivate(CMD_UI_POP);
		if (alignToNode == nullptr)
			return;

		alignTo(alignToNode, NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT);
	}

	virtual ~UINodeBackButton ()
	{
	}
};
