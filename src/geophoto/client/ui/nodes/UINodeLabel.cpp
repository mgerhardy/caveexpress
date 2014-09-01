#include "UINodeLabel.h"
#include "client/ui/BitmapFont.h"

UINodeLabel::UINodeLabel (IFrontend *frontend, const std::string& label) :
		UINode(frontend), _label(label)
{
	_wantFocus = UINode::wantFocus();
	_font = getFont();
	Vector4Set(colorBlack, _fontColor);
	_textWidth = _font->getTextWidth(_label);
	_textHeight = _font->getTextHeight(_label);
	autoSize();
}

UINodeLabel::UINodeLabel (IFrontend *frontend, const std::string& label, const BitmapFontPtr& font) :
		UINode(frontend), _label(label), _font(font)
{
	_wantFocus = UINode::wantFocus();
	Vector4Set(colorBlack, _fontColor);
	_textWidth = _font->getTextWidth(_label);
	_textHeight = _font->getTextHeight(_label);
	autoSize();
}

UINodeLabel::~UINodeLabel ()
{
}

void UINodeLabel::setFont (const std::string& font)
{
	_font = getFont(font);
	_textWidth = _font->getTextWidth(_label);
	_textHeight = _font->getTextHeight(_label);
	autoSize();
}

bool UINodeLabel::wantFocus () const
{
	return _wantFocus;
}

void UINodeLabel::setLabel (const std::string& label)
{
	_label = label;
	_textWidth = _label.size() * _font->getCharWidth();
	autoSize();
}

float UINodeLabel::getAutoWidth () const
{
	return _textWidth / static_cast<float>(_frontend->getWidth()) + getPadding() * 2.0f;
}

float UINodeLabel::getAutoHeight () const
{
	return _textHeight / static_cast<float>(_frontend->getHeight()) + getPadding() * 2.0f;
}

void UINodeLabel::render (int x, int y) const
{
	UINode::render(x, y);
	if (_label.empty())
		return;
	x += getRenderCenterX() - _textWidth / 2;
	y += getRenderCenterY() - _font->getTextHeight(_label) / 2;
	_font->print(_label, _fontColor, x, y);
}
