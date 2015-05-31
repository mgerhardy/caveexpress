#pragma once

#include "client/ui/nodes/UINodeBackground.h"

class UINodeMainBackground: public UINodeBackground {
private:
	TexturePtr _mainTitle;
	bool _renderTitle;
public:
	UINodeMainBackground (IFrontend *frontend, bool renderTitle = true) :
			UINodeBackground(frontend, "", false), _renderTitle(renderTitle)
	{
		_mainTitle = loadTexture("ui-background-title");
	}

	void renderMiddle (int x, int y) const override
	{
		UINodeBackground::renderMiddle(x, y);
		if (!_renderTitle)
			return;
		renderImage(_mainTitle, x + getRenderWidth(false) / 2 - _mainTitle->getWidth() / 2, y);
	}
};
