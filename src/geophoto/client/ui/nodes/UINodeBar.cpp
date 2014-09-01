#include "UINodeBar.h"
#include <assert.h>

UINodeBar::UINodeBar (IFrontend *frontend) :
		UINode(frontend), _max(0), _current(0)
{
	_colorBackground[0] = 0.5f;
	_colorBackground[1] = 0.5f;
	_colorBackground[2] = 0.5f;
	_colorBackground[3] = 1.0f;
	_colorBar[0] = 0.0f;
	_colorBar[1] = 0.8f;
	_colorBar[2] = 0.0f;
	_colorBar[3] = 0.5f;
	setBorder(true);
}

UINodeBar::~UINodeBar ()
{
}

void UINodeBar::setBarColor (const Color& color)
{
	Vector4Set(color, _colorBar);
}

void UINodeBar::setBackgroundColor (const Color& color)
{
	Vector4Set(color, _colorBackground);
}

void UINodeBar::render (int x, int y) const
{
	UINode::render(x, y);

	const int w = getRenderWidth(false);
	const int h = getRenderHeight(false);
	x += getRenderX(false);
	y += getRenderY(false);

	renderFilledRect(x, y, w, h, _colorBackground);

	if (_max > 0 && _current > 0) {
		const float factor = static_cast<float>(_current) / static_cast<float>(_max);
		assert(factor <= 1.0f + EPSILON);
		const int width = w * factor;
		if (width > 0) {
			renderFilledRect(x, y, width, h, _colorBar);
		}
	}

	if (_renderBorder) {
		renderRect(x, y, w, h, _borderColor);
	}
}

void UINodeBar::setMax (int max)
{
	_max = max;
}

void UINodeBar::setCurrent (int current)
{
	_current = current;
}
