#pragma once

#include "UINodeButton.h"

class UINodeButtonImage: public UINodeButton {
public:
	UINodeButtonImage (IFrontend *frontend, const std::string& image) :
			UINodeButton(frontend)
	{
		setImage(image);
		if (System.supportFocusChange()) {
			setAlpha(0.5f);
			setFocusAlpha(1.0f);
		}
	}

	virtual ~UINodeButtonImage ()
	{
	}
};
