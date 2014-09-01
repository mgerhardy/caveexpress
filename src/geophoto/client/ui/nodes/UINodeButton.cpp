#include "UINodeButton.h"
#include "shared/Logger.h"

UINodeButton::UINodeButton (IFrontend *frontend, const std::string& title) :
		UINode(frontend), _title(title), _titleAlign(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE)
{
	_font = getFont();
	Vector4Set(colorBlack, _fontColor);
	setFocusAlpha(0.5f);
	setStandardPadding();
	if (!title.empty())
		autoSize();
}

UINodeButton::~UINodeButton ()
{
}

bool UINodeButton::wantFocus () const
{
	return true;
}

void UINodeButton::setTitleAlignment (int align)
{
	_titleAlign = align;
}

void UINodeButton::render (int x, int y) const
{
	UINode::render(x, y);
	if (_title.empty())
		return;

	if (_titleAlign & NODE_ALIGN_CENTER) {
		x += getRenderCenterX() - _font->getTextWidth(_title) / 2;
	} else if (_titleAlign & NODE_ALIGN_RIGHT) {
		x += getRenderWidth() - _font->getTextWidth(_title);
	} else {
		x += getRenderCenterX() - _font->getTextWidth(_title) / 2;
	}

	if (_titleAlign & NODE_ALIGN_MIDDLE) {
		y += getRenderCenterY() - _font->getTextHeight(_title) / 2;
	} else if (_titleAlign & NODE_ALIGN_BOTTOM) {
		y += getRenderY() + getRenderHeight() - _font->getTextHeight(_title);
	} else {
		y += getRenderCenterY() - _font->getTextHeight(_title) / 2;
	}

	_font->print(_title, _fontColor, x, y);
}

float UINodeButton::getAutoWidth () const
{
	const float w = std::max(UINode::getAutoWidth(),
			_font->getTextWidth(_title) / static_cast<float>(_frontend->getWidth()) + 2.0f * getPadding());
	return w;
}

float UINodeButton::getAutoHeight () const
{
	const float h = std::max(UINode::getAutoHeight(),
			_font->getTextHeight(_title) / static_cast<float>(_frontend->getHeight()) + 2.0f * getPadding());
	return h;
}
