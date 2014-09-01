#pragma once

#include "UINode.h"
#include "client/ui/BitmapFont.h"

class UINodeSpinner: public UINode {
private:
	BitmapFontPtr _font;
	Color _fontColor;
	int _value;
	const int _min;
	int _max;

public:
	UINodeSpinner (IFrontend *frontend, int min = 0, int max = 1);
	virtual ~UINodeSpinner ();

	void setValue (int value);
	int getValue () const;

	void setMax (int max);

	void increaseValue ();
	void decreaseValue ();

	// UINode
	bool wantFocus () const override;
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
}
