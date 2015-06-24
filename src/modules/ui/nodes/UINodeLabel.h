#pragma once

#include "UINode.h"

class UINodeLabel: public UINode {
protected:
	std::string _label;
	BitmapFontPtr _font;
	Color _fontColor;
	int _textWidth;
	int _textHeight;

public:
	UINodeLabel (IFrontend *frontend, const std::string& label);
	UINodeLabel (IFrontend *frontend, const std::string& label, const BitmapFontPtr& font);
	virtual ~UINodeLabel ();

	virtual void setLabel (const std::string& label);

	void setFont (const std::string& font);
	void setColor (const Color& color);

	// UINode
	float getAutoWidth () const override;
	float getAutoHeight () const override;
	void render (int x, int y) const override;
};
