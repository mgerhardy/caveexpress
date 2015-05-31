#pragma once

#include "UINode.h"
#include "client/ui/BitmapFont.h"

class UINodeTextInput: public UINode {
private:
	BitmapFontPtr _font;
	Color _fontColor;
	bool _handleInput;
	std::string _text;
	std::string _input;
	mutable int _frame;
	mutable bool _cursorBlink;

	void setHandleInput (bool handleInput);
public:
	UINodeTextInput (IFrontend *frontend, const std::string& font = DEFAULT_FONT, int minChars = 8);
	virtual ~UINodeTextInput ();

	const std::string& getValue () const;
	void setValue (const std::string& value);

	// UINode
	bool onTextInput (const std::string& text) override;
	float getAutoWidth () const override;
	float getAutoHeight () const override;
	void render (int x, int y) const override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;
	bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button) override;
	void removeFocus () override;
	// override this to not pop a window if we are in edit mode
	bool onPop () override;
};

inline const std::string& UINodeTextInput::getValue () const
{
	return _text;
}

inline void UINodeTextInput::setValue (const std::string& value)
{
	_input = "";
	_text = value;
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onValueChanged();
	}
}
