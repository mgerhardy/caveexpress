#include "UINodeArrow.h"

UINodeArrow::UINodeArrow (IFrontend *frontend) :
		UINode(frontend), _startX(0.0f), _startY(0.0f), _endX(0.0f), _endY(0.0f)
{
	Vector4Set(colorWhite, _color);
}

UINodeArrow::~UINodeArrow ()
{
}

void UINodeArrow::render (int x, int y) const
{
	UINode::render(x, y);
	const float w = _frontend->getWidth();
	const float h = _frontend->getHeight();
	_frontend->renderLine(_startX * w, _startY * h, _endX * w, _endY * h, _color);
}
