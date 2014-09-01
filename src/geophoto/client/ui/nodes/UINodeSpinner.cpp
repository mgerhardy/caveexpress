#include "UINodeSpinner.h"
#include "shared/Logger.h"

UINodeSpinner::UINodeSpinner (IFrontend *frontend, int min, int max) :
		UINode(frontend), _value(0), _min(min), _max(max)
{
	_font = getFont();
	Vector4Set(colorBlack, _fontColor);
	setStandardPadding();
	_renderBorder = true;
}

UINodeSpinner::~UINodeSpinner ()
{
}

void UINodeSpinner::setValue (int value)
{
	_value = value;
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onValueChanged();
	}
}

void UINodeSpinner::render (int x, int y) const
{
	UINode::render(x, y);
	const int buttonWidth = 40;
	const int minusButtonX = x + getRenderX() + buttonWidth / 2 - _font->getCharWidth() / 2;
	const int plusButtonX = x + getRenderX() + getRenderWidth() - buttonWidth / 2 -  _font->getCharWidth() / 2;
	const std::string valStr = string::toString(_value);
	const int valueX = x + getRenderCenterX() - valStr.length() * _font->getCharWidth();
	y += getRenderCenterY() - _font->getCharHeight() / 2;
	_font->print("-", _fontColor, minusButtonX, y);
	_font->print("+", _fontColor, plusButtonX, y);
	_font->print(valStr, _fontColor, valueX, y);
}

void UINodeSpinner::handleDrop (uint16_t x, uint16_t y)
{
	const int relX = x - getRenderX();
	const int halfWidth = getRenderWidth() / 2;
	if (relX > halfWidth)
		increaseValue();
	else
		decreaseValue();
}

void UINodeSpinner::increaseValue ()
{
	const int value = getValue() + 1;
	if (value > _max)
		return;
	setValue(value);
}

void UINodeSpinner::decreaseValue ()
{
	const int value = getValue() - 1;
	if (value < _min)
		return;
	setValue(value);
}

bool UINodeSpinner::wantFocus () const
{
	return true;
}
