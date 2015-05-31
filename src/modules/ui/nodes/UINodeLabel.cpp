#include "UINodeLabel.h"
#include "ui/BitmapFont.h"

UINodeLabel::UINodeLabel (IFrontend *frontend, const std::string& label) :
		UINode(frontend, label), _label(label)
{
	_font = getFont();
	Vector4Set(colorBlack, _fontColor);
	_textWidth = _font->getTextWidth(_label);
	_textHeight = _font->getTextHeight(_label);
	autoSize();
}

UINodeLabel::UINodeLabel (IFrontend *frontend, const std::string& label, const BitmapFontPtr& font) :
		UINode(frontend, label), _label(label), _font(font)
{
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

void UINodeLabel::setColor (const Color& color)
{
	Vector4Set(color, _fontColor);
}

void UINodeLabel::setLabel (const std::string& label)
{
	_label = label;
	_textWidth = _font->getTextWidth(_label);
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

void UINodeLabel::printValues (std::ostream &stream) const
{
	stream << "label: " << _label << ", ";
	stream << "textWidth: " << _textWidth << ", ";
	stream << "font: " << _font->getId() << ", ";
	stream << "fontColor: " << _fontColor[0] << ":" << _fontColor[1] << ":" << _fontColor[2] << ":" << _fontColor[3] << ", ";
}

std::string UINodeLabel::getPrintNodeName () const
{
	return "UINodeLabel";
}
