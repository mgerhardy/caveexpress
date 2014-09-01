#pragma once

#include "UINode.h"

class UINodeLabel: public UINode {
protected:
	std::string _label;
	BitmapFontPtr _font;
	Color _fontColor;
	int _textWidth;
	int _textHeight;
	bool _wantFocus;

public:
	UINodeLabel (IFrontend *frontend, const std::string& label);
	UINodeLabel (IFrontend *frontend, const std::string& label, const BitmapFontPtr& font);
	virtual ~UINodeLabel ();

	void setLabel (const std::string& label);
	void setWantFocus (bool focus);

	void setFont (const std::string& font);

	// UINode
	bool wantFocus () const override;
	float getAutoWidth () const override;
	float getAutoHeight () const override;
	void render (int x, int y) const override;
};

inline void UINodeLabel::setWantFocus (bool focus)
{
	_wantFocus = focus;
}
