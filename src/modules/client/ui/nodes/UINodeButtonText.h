#pragma once

#include "UINodeButton.h"

#define DEFAULT_PADDING 0.01f

class UINodeButtonText: public UINodeButton {
public:
	UINodeButtonText (IFrontend *frontend, const std::string& title, float padding = DEFAULT_PADDING) :
			UINodeButton(frontend, title)
	{
		setBackgroundColor(colorWhite);
		setBorder(true);
		setBorderColor(colorBlack);
		setPadding(padding);
		if (System.supportFocusChange()) {
			setAlpha(0.5f);
			setFocusAlpha(1.0f);
		}
		autoSize();
	}

	virtual ~UINodeButtonText ()
	{
	}
};
