#include "UINodeSpinner.h"
#include "engine/common/Logger.h"

namespace {
const int gap = 20;
}

UINodeSpinner::UINodeSpinner (IFrontend *frontend, int min, int max, int stepWidth) :
		UINode(frontend), _value(0), _min(min), _max(max), _stepWidth(stepWidth)
{
	_font = getFont();
	Vector4Set(colorBlack, _fontColor);
	setStandardPadding();
	_renderBorder = true;
}

UINodeSpinner::~UINodeSpinner ()
{
}

float UINodeSpinner::getAutoWidth () const
{
	const std::string valStr = string::toString(_max);
	const int bw = _font->getCharWidth() * 2 + _font->getTextWidth(valStr) + 2 * gap;
	return bw / static_cast<float>(_frontend->getWidth());
}

void UINodeSpinner::setValue (int value)
{
	if (value == _value)
		return;

	_value = value;
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onValueChanged();
	}
}

void UINodeSpinner::render (int x, int y) const
{
	UINode::render(x, y);
	const std::string valStr = string::toString(_value);
	const std::string maxValStr = string::toString(_max);
	const int rx = getRenderX();
	const int minusButtonX = x + rx;
	const int plusButtonX = x + getRenderX(false) + getRenderWidth(true) - _font->getCharWidth();
	const int minusButtonXRight = minusButtonX + _font->getCharWidth();
	const int space = plusButtonX - minusButtonXRight;
	const int valueX = minusButtonXRight + space / 2 - _font->getTextWidth(valStr) / 2;
	y += getRenderCenterY() - _font->getCharHeight() / 2;
	_font->print("M", _fontColor, minusButtonX, y);
	_font->print("P", _fontColor, plusButtonX, y);
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
	const int value = getValue() + _stepWidth;
	if (value > _max)
		return;
	setValue(value);
}

void UINodeSpinner::decreaseValue ()
{
	const int value = getValue() - _stepWidth;
	if (value < _min)
		return;
	setValue(value);
}

bool UINodeSpinner::onMouseWheel (int32_t x, int32_t y)
{
	const bool retVal = UINode::onMouseWheel(x, y);
	if (y > 0)
		increaseValue();
	else
		decreaseValue();
	return retVal;
}
