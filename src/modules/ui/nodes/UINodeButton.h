#pragma once

#include "UINode.h"
#include "ui/BitmapFont.h"

class UINodeButton: public UINode {
protected:
	std::string _title;
	int _titleAlign;
	BitmapFontPtr _font;
	Color _fontColor;
	uint32_t _triggerTimeMs;
	uint32_t _lastTriggerTimeMs;
public:
	UINodeButton (IFrontend *frontend, const std::string& title = "");
	virtual ~UINodeButton ();

	/**
	 * @brief Sets a trigger time for reexecuting the attached action if a finger or button is kept pressed.
	 * By setting this to <= 0 the reactivation is disabled.
	 */
	void setTriggerTime(uint32_t ms) {
		_triggerTimeMs = ms;
	}

	void setTitleAlignment (int align);
	void setTitle (const std::string& title);
	void setFont (const BitmapFontPtr& font, const Color& color = colorBlack);
	inline void setColor (const Color& color = colorBlack) { Vector4Set(color, _fontColor); }

	virtual void update (uint32_t deltaTime) override;
	virtual float getAutoWidth () const override;
	virtual float getAutoHeight () const override;
	virtual bool isActive () const override;
	virtual void render (int x, int y) const override;
};

inline void UINodeButton::setTitle (const std::string& title)
{
	_title = title;
	_id = title;
	if (!_title.empty())
		autoSize();
}

inline void UINodeButton::setFont (const BitmapFontPtr& font, const Color& color)
{
	_font = font;
	Vector4Set(color, _fontColor);
	if (!_title.empty())
		autoSize();
}

inline bool UINodeButton::isActive () const
{
	return isVisible();
}
