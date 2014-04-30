#pragma once

#include "caveexpress/client/ui/nodes/UINodeBackground.h"

class UINodePopupBackground: public UINodeBackground {
public:
	UINodePopupBackground (IFrontend *frontend, const std::string& title) :
		UINodeBackground(frontend, title, false)
	{
		setAmount(2, 1);
		setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
		setBorder(true);
		setBorderColor(colorWhite);
	}

	virtual TexturePtr getCave () const
	{
		return _tiles[0];
	}
};
