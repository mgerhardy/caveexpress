#pragma once

#include "UINodeBackground.h"

class UINodeSettingsBackground: public UINodeBackground {
public:
	UINodeSettingsBackground (IFrontend *frontend, const std::string& title = tr("Settings")) :
			UINodeBackground(frontend, title, false)
	{
	}

	TexturePtr getCave () const override
	{
		return _tiles[0];
	}

	TexturePtr getCaveArt () const override
	{
		return _tiles[1];
	}
};
