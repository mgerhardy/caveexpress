#pragma once

#include "IUILayout.h"

class UIVBoxLayout: public IUILayout {
private:
	float _spacing;
	bool _expandChildren;
	int _align;
public:
	UIVBoxLayout (float spacing = 0.0f, bool expandChildren = false, int align = 0);
	virtual ~UIVBoxLayout ();

	void setSpacing (float spacing);

	void layout (UINode* parent) override;
};

inline void UIVBoxLayout::setSpacing (float spacing)
{
	_spacing = spacing;
}
