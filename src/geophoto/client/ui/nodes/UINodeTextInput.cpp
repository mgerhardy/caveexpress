#include "UINodeTextInput.h"
#include <SDL.h>

UINodeTextInput::UINodeTextInput (IFrontend *frontend, float minWidth) :
		UINode(frontend), _handleInput(false), _frame(0), _cursorBlink(false)
{
	_font = getFont();
	Vector4Set(colorBlack, _fontColor);
	setBorder(true);
	setBorderColor(colorWhite);
	setMinWidth(minWidth);
	setStandardPadding();
	autoSize();
}

UINodeTextInput::~UINodeTextInput ()
{
}

float UINodeTextInput::getAutoWidth () const
{
	const float w = _font->getTextWidth(_text) / static_cast<float>(_frontend->getWidth()) + 2.0f * getPadding();
	return w;
}

float UINodeTextInput::getAutoHeight () const
{
	const float h = _font->getTextHeight(_text) / static_cast<float>(_frontend->getHeight()) + 2.0f * getPadding();
	return h;
}

void UINodeTextInput::render (int x, int y) const
{
	UINode::render(x, y);
	std::string renderStr = _text + _input;

	_frame++;
	if ((_frame % 10) == 0)
		_cursorBlink ^= true;
	if (_handleInput) {
		if (_cursorBlink)
			renderStr += "_";
		else if (!renderStr.empty())
			renderStr += " ";
	}
	if (renderStr.empty())
		return;
	const int maxChars = getRenderWidth() / _font->getCharWidth();
	const int length = renderStr.size();
	const int textWidth = length * _font->getCharWidth();
	if (textWidth > getRenderWidth()) {
		if (_handleInput) {
			// we show the end of the string
			renderStr = renderStr.substr(length - maxChars, maxChars);
		} else {
			// we show the beginning of the string
			renderStr = renderStr.substr(0, maxChars);
		}
	}

	x += getRenderX();
	y += getRenderY();
	_font->print(renderStr, _fontColor, x, y);
}

bool UINodeTextInput::onKeyPress (int32_t key, int16_t modifier)
{
	if (!_handleInput)
		return false;

	if (key == SDLK_KP_ENTER || key == SDLK_RETURN) {
		_text += _input;
		_input = "";
		_handleInput = false;
		for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
			(*i)->onValueChanged();
		}
	} else if (key == SDLK_ESCAPE) {
		_input = "";
		_handleInput = false;
	} else if (key == SDLK_BACKSPACE) {
		if (!_input.empty())
			_input.resize(_input.size() - 1);
		else if (!_text.empty())
			_text.resize(_text.size() - 1);
	} else {
		_input.push_back(key);
	}
	return true;
}

bool UINodeTextInput::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	_handleInput = true;
	return true;
}

bool UINodeTextInput::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	_handleInput = true;
	return true;
}

bool UINodeTextInput::wantFocus () const
{
	return true;
}
