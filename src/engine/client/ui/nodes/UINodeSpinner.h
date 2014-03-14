#pragma once

#include "UINode.h"
#include "engine/client/ui/BitmapFont.h"

class UINodeSpinner: public UINode {
private:
	BitmapFontPtr _font;
	Color _fontColor;
	int _value;
	const int _min;
	int _max;
	int _stepWidth;

public:
	UINodeSpinner (IFrontend *frontend, int min = 0, int max = 1, int stepWidth = 1);
	virtual ~UINodeSpinner ();

	void setValue (int value);
	int getValue () const;

	void setMax (int max);

	void setStepWidth (int stepWidth);

	void increaseValue ();
	void decreaseValue ();

	// UINode
	float getAutoWidth () const override;
	bool onMouseWheel (int32_t x, int32_t y) override;
	void render (int x, int y) const override;
	void handleDrop (uint16_t x, uint16_t y) override;
};

inline int UINodeSpinner::getValue () const
{
	return _value;
}

inline void UINodeSpinner::setMax (int max)
{
	_max = max;
	autoSize();
}

inline void UINodeSpinner::setStepWidth (int stepWidth)
{
	_stepWidth = stepWidth;
}
