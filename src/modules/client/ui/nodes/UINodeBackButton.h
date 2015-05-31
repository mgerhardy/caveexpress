#pragma once

#include "client/ui/nodes/UINodeButtonImage.h"
#include "common/Commands.h"

class UINodeBackButton: public UINodeButtonImage {
public:
	UINodeBackButton (IFrontend *frontend, UINode* alignToNode = nullptr) :
		UINodeButtonImage(frontend, "icon-arrow-left")
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
