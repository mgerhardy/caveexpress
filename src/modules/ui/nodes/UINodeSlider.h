#pragma once

#include "UINode.h"

class UINodeSlider: public UINode {
private:
	float _value;
	const float _min;
	float _max;
	const float _stepWidth;
	Color _lineColor;
	Color _sliderColor;

	float calculateValue (int32_t x) const;

public:
	UINodeSlider (IFrontend *frontend, float min = 0.0f, float max = 1.0f, float stepWidth = 1.0f);
	virtual ~UINodeSlider ();

	void setColors (const Color& lineColor, const Color& sliderColor);
	void setValue (float value);
	float getValue () const;

	void setMax (float max);

	// UINode
	void render (int x, int y) const override;
	bool onMouseWheel (int32_t x, int32_t y) override;
	bool onJoystickMotion (bool horizontal, int value) override;
	bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y, bool motion) override;
	bool onMouseLeftRelease (int32_t x, int32_t y) override;
};

inline float UINodeSlider::getValue () const
{
	return _value;
}

inline void UINodeSlider::setMax (float max)
{
	_max = max;
	if (_value > _max)
		setValue(_max);
}
