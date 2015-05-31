#pragma once

#include "client/ui/nodes/UINode.h"
#include "client/ui/nodes/UINodeBackground.h"

class UINodeGameOverBackground: public UINodeBackground {
private:
	TexturePtr _gameover;
public:
	UINodeGameOverBackground (IFrontend *frontend) :
			UINodeBackground(frontend, "", false)
	{
		_gameover = loadTexture("ui-background-gameover");
	}

	void renderMiddle (int x, int y) const override
	{
		UINodeBackground::renderMiddle(x, y);
		renderImage(_gameover, x + getRenderCenterX() - _gameover->getWidth() / 2, y + getRenderCenterY() - _gameover->getHeight() / 2);
	}
};
