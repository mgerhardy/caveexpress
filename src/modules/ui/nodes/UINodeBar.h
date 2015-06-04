#pragma once

#include "UINode.h"

class UINodeBar: public UINode {
private:
	int _max;
	int _current;
	Color _colorBackground;
	Color _colorBar;

public:
	explicit UINodeBar (IFrontend *frontend);
	virtual ~UINodeBar ();

	void setMax (int max);
	void setCurrent (int current);

	int getMax () const;
	int getCurrent () const;

	void setBarColor (const Color& color);
	void setBackgroundColor (const Color& color);

	void render (int x, int y) const override;
};

inline int UINodeBar::getMax () const
{
	return _max;
}

inline int UINodeBar::getCurrent () const
{
	return _current;
}
