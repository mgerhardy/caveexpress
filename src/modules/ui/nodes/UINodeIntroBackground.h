#pragma once

#include "ui/nodes/UINodeBackground.h"

class UINodeIntroBackground: public UINodeBackground {
public:
	UINodeIntroBackground (IFrontend *frontend, int width = 2, int height = 2) :
			UINodeBackground(frontend, "", false)
	{
		setAmount(width, height);
		setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
	}

	TexturePtr getCave () const override {
		return _tiles[0];
	}
};
