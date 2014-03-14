#pragma once

#include "engine/client/ui/nodes/UINode.h"

class UINodePopupBackground: public UINode {
public:
	UINodePopupBackground (IFrontend *frontend) :
			UINode(frontend)
	{
		setBackgroundColor(colorBlack);
		_texture = loadTexture("ui-scene-tile1-ice");
		setSize(_texture->getWidth() / static_cast<float>(frontend->getWidth()), _texture->getHeight() / static_cast<float>(frontend->getHeight()));
		setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
		setBorder(true);
		setBorderColor(colorWhite);
	}
};
