#include "UINodeTextInput.h"
#include "engine/common/Logger.h"
#include <SDL.h>

UINodeTextInput::UINodeTextInput (IFrontend *frontend, const std::string& font, int minChars) :
		UINode(frontend), _handleInput(false), _frame(0), _cursorBlink(false)
{
	_font = getFont(font);
	Vector4Set(colorBlack, _fontColor);
	setBorder(true);
	setBorderColor(colorBlack);
	setBackgroundColor(colorWhite);
	setMinWidth((minChars * _font->getTextWidth("A")) / static_cast<float>(_frontend->getWidth()));
	setStandardPadding();
	autoSize();
}

UINodeTextInput::~UINodeTextInput ()
{
}

bool UINodeTextInput::onTextInput (const std::string& text)
{
	_text.append(text);
	return true;
}

void UINodeTextInput::setHandleInput (bool handleInput)
{
	if (handleInput) {
		SDL_StartTextInput();
		setBackgroundColor(colorGray);
	} else {
		SDL_StopTextInput();
		setBackgroundColor(colorWhite);
		UINode::removeFocus();
	}
	_handleInput = SDL_IsTextInputActive();
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
	const int maxChars = _font->getMaxCharsForLength(renderStr, getRenderWidth(false));
	if (maxChars <= 0)
		return;
	const int length = renderStr.size();
	if (maxChars < length) {
		if (_handleInput) {
			// we show the end of the string
			renderStr = renderStr.substr(length - 1 - maxChars, maxChars);
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
		setHandleInput(false);
		for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
			(*i)->onValueChanged();
		}
		return true;
	} else if (key == SDLK_ESCAPE) {
		_input = "";
		setHandleInput(false);
		return true;
	} else if (key == SDLK_BACKSPACE) {
		if (!_input.empty())
			_input.resize(_input.size() - 1);
		else if (!_text.empty())
			_text.resize(_text.size() - 1);
	}

	if (key >= ' ' && key <= '~') {
		if (!SDL_IsTextInputActive()) {
			_input.push_back(key);
			return true;
		}
	}

	return false;
}

bool UINodeTextInput::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	setHandleInput(true);
	return true;
}

bool UINodeTextInput::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	setHandleInput(true);
	return true;
}

void UINodeTextInput::removeFocus ()
{
	// focus is removed with the end of the editing
	if (_handleInput)
		return;

	UINode::removeFocus();
}

bool UINodeTextInput::onPop ()
{
	if (_handleInput)
		return false;
	return UINode::onPop();
}
