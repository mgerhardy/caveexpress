#include "UINodeButton.h"
#include "common/Log.h"

namespace {
const float BUTTON_ALPHA = 0.5f;
const float BUTTON_FOCUS_ALPHA = 1.0f;
}

UINodeButton::UINodeButton (IFrontend *frontend, const std::string& title) :
		UINode(frontend, title), _title(title), _titleAlign(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE), _triggerTimeMs(0u), _lastTriggerTimeMs(0u)
{
	setAlpha(BUTTON_ALPHA);
	_font = getFont();
	Vector4Set(colorBlack, _fontColor);
	setFocusAlpha(BUTTON_FOCUS_ALPHA);
	setStandardPadding();
	if (!title.empty())
		autoSize();
}

UINodeButton::~UINodeButton ()
{
}

void UINodeButton::setTitleAlignment (int align)
{
	_titleAlign = align;
}

void UINodeButton::render (int x, int y) const
{
	// TODO: fix _enabled rendering - should get grayed or something like that if it's disabled
	UINode::render(x, y);
	if (_title.empty())
		return;

	if (_texture)
		x += _texture->getWidth();

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

void UINodeButton::update (uint32_t deltaTime) {
	UINode::update(deltaTime);

	if (_triggerTimeMs <= 0u)
		return;

	if (_focus && (_fingerPressed || _mousePressed)) {
		if (_time - _lastTriggerTimeMs > _triggerTimeMs) {
			_lastTriggerTimeMs = _time;
			execute();
		}
	}
}

float UINodeButton::getAutoWidth () const
{
	const float w = UINode::getAutoWidth() + _font->getTextWidth(_title) / static_cast<float>(_frontend->getWidth()) + 2.0f * getPadding();
	return w;
}

float UINodeButton::getAutoHeight () const
{
	const float h = std::max(UINode::getAutoHeight(),
			_font->getTextHeight(_title) / static_cast<float>(_frontend->getHeight()) + 2.0f * getPadding());
	return h;
}
