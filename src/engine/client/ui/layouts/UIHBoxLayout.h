#pragma once

#include "IUILayout.h"

class UIHBoxLayout: public IUILayout {
private:
	bool _expandChildren;
	float _spacing;
public:
	UIHBoxLayout (float spacing = 0.0f, bool expandChildren = false);
	virtual ~UIHBoxLayout ();

	void setSpacing (float spacing);

	void layout (UINode* parent) override;
};

inline void UIHBoxLayout::setSpacing (float spacing)
{
	_spacing = spacing;
}
