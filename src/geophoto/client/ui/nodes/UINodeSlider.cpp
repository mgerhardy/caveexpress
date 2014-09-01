#include "UINodeSlider.h"
#include "shared/Logger.h"

UINodeSlider::UINodeSlider (IFrontend *frontend, float min, float max, float stepWidth) :
	UINode(frontend), _value(0), _min(min), _max(max), _stepWidth(stepWidth)
{
	setColors(colorWhite, colorWhite);
	setStandardPadding();
}

UINodeSlider::~UINodeSlider ()
{
}

void UINodeSlider::setColors (const Color& lineColor, const Color& sliderColor)
{
	Vector4Set(lineColor, _lineColor);
	Vector4Set(sliderColor, _sliderColor);
}

void UINodeSlider::render (int x, int y) const
{
	UINode::render(x, y);
	x += getRenderX();
	y += getRenderY();
	const int w = getRenderWidth();
	const int h = getRenderHeight();
	const int deltaHeight = h / 2;
	const float steps = _max - _min + 1.0f;
	const float stepDelta = w / steps * (_stepWidth < 1.0f ? 1.0f : _stepWidth);
	const int sliderX = x + (_value - _min) / steps * w;
	renderLine(x, y + deltaHeight, x + w, y + deltaHeight, _lineColor);
	renderFilledRect(sliderX, y, stepDelta, h, _sliderColor);
}

inline float UINodeSlider::calculateValue (int32_t x) const
{
	const int steps = _max - _min + 1;
	const float value = _min + (x - getRenderX(false)) * steps / static_cast<float>(getRenderWidth(false));
	return clamp(value - static_cast<float>(fmod(value, _stepWidth)), _min, _max);
}

void UINodeSlider::handleDrop (uint16_t x, uint16_t y)
{
	_value = calculateValue(x);
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onValueChanged();
	}
	UINode::handleDrop(x, y);
}

bool UINodeSlider::wantFocus () const
{
	return true;
}
