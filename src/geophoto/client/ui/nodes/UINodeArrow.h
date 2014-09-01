#pragma once

#include "UINode.h"

class UINodeArrow: public UINode {
private:
	float _startX;
	float _startY;
	float _endX;
	float _endY;
	Color _color;
public:
	UINodeArrow (IFrontend *frontend);
	virtual ~UINodeArrow ();

	void setStart (float x, float y);
	void setEnd (float x, float y);
	void setColor (const Color& color);

	void render (int x, int y) const override;
};

inline void UINodeArrow::setStart (float x, float y)
{
	_startX = x;
	_startY = y;
}

inline void UINodeArrow::setEnd (float x, float y)
{
	_endX = x;
	_endY = y;
}

inline void UINodeArrow::setColor (const Color& color)
{
	Vector4Set(color, _color);
}
