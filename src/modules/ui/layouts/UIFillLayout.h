#pragma once

#include "IUILayout.h"

/**
 * FillLayout is the simplest layout class. It lays out
 * controls in a single row or column, forcing them to be the same size.
 */
class UIFillLayout: public IUILayout {
protected:
	bool _horizontal;
public:
	explicit UIFillLayout (bool horizontal = true);
	virtual ~UIFillLayout ();

	void layout (UINode* parent) override;
};
