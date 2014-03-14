#include "UINodeCheckbox.h"
#include "engine/common/Logger.h"

UINodeCheckbox::UINodeCheckbox (IFrontend *frontend, const std::string& id, const std::string& icon) :
		UINodeButton(frontend), _value(false)
{
	setId(id);
	setBackground(icon);
}

UINodeCheckbox::~UINodeCheckbox ()
{
}

void UINodeCheckbox::setSelected (bool value)
{
	if (_value == value)
		return;
	_value = value;
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onValueChanged();
	}
	updateImage();
}

void UINodeCheckbox::updateImage ()
{
	if (_value)
		_texture = _checkboxOn;
	else
		_texture = _checkboxOff;
}

bool UINodeCheckbox::isSelected ()
{
	return _value;
}

void UINodeCheckbox::setBackground (const std::string& background)
{
	_checkboxOn = loadTexture(background + "-on");
	_checkboxOff = loadTexture(background + "-off");
	updateImage();
	float w = getWidth();
	if (w <= 0.0f)
		w = getAutoWidth();
	float h = getHeight();
	if (h <= 0.0f)
		h = getAutoHeight();
	setSize(w, h);
}

bool UINodeCheckbox::onFingerRelease (int64_t finger, uint16_t x, uint16_t y)
{
	if (!_enabled)
		return false;

	setSelected(_value ^ true);
	return UINodeButton::onFingerRelease(finger, x, y);
}

bool UINodeCheckbox::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	if (!_enabled)
		return false;

	setSelected(_value ^ true);
	return UINode::onMouseButtonRelease(x, y, button);
}

void UINodeCheckbox::render (int x, int y) const
{
	UINodeButton::render(x, y);

	if (_label.empty())
		return;

	x += getRenderX() + getRenderWidth() + getPadding() * _frontend->getWidth();
	y += getRenderCenterY() - _font->getTextHeight(_label) / 2;
	_font->print(_label, _fontColor, x, y);
}
