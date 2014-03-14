#pragma once

#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/BitmapFont.h"

class UINodeMainButton: public UINodeButton {
public:
	UINodeMainButton(IFrontend *frontend, const std::string& title, const std::string& font = LARGE_FONT, const Color& color = colorWhite) :
			UINodeButton(frontend, title) {
		setFont(getFont(font), color);
		setTitleAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
		autoSize();
	}

	void addFocus (int32_t x = -1, int32_t y = -1) override
	{
		UINodeButton::addFocus(x, y);
		setBackgroundColor(colorGrayAlpha40);
	}

	void removeFocus ()
	{
		UINodeButton::removeFocus();
		setBackgroundColor(colorNull);
	}

	virtual ~UINodeMainButton() {
	}
};
