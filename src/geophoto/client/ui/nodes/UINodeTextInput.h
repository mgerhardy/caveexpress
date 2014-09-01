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
public:
	UINodeTextInput (IFrontend *frontend, float minWidth = 0.0f);
	virtual ~UINodeTextInput ();

	const std::string& getValue () const;
	void setValue (const std::string& value);

	// UINode
	float getAutoWidth () const override;
	float getAutoHeight () const override;
	bool wantFocus () const override;
	void render (int x, int y) const override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;
	bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button) override;
};

inline const std::string& UINodeTextInput::getValue () const
{
	return _text;
}

inline void UINodeTextInput::setValue (const std::string& value)
{
	_input = "";
	_text = value;
}
