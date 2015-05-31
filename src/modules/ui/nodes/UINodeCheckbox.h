#pragma once

#include "UINodeButton.h"
#include "common/String.h"

class UINodeCheckbox: public UINodeButton {
protected:
	bool _value;
	TexturePtr _checkboxOn;
	TexturePtr _checkboxOff;
	std::string _label;

	void updateImage ();

public:
	UINodeCheckbox (IFrontend *frontend, const std::string& id, const std::string& icon = "icon-checkbox");
	virtual ~UINodeCheckbox ();

	void setTitleAlignment (int align);
	void setSelected (bool value);
	bool isSelected ();
	void setBackground (const std::string& background);
	void setLabel (const std::string& label);

	// UINode
	void render (int x, int y) const override;
	bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y) override;
	bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button) override;
};

inline void UINodeCheckbox::setLabel (const std::string& label)
{
	_label = label;
}
