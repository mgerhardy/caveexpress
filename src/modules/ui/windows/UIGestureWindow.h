#pragma once

#include "ui/windows/UIWindow.h"
#include "common/vec2.h"

#include <vector>

class UIGestureWindow: public UIWindow {
private:
	typedef std::vector<vec2> Coords;
	typedef Coords::const_iterator CoordsConstIter;
	Coords _coords;
public:
	explicit UIGestureWindow (IFrontend *frontend);
	void onActive () override;
	void render (int x, int y) const override;
	bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override;
	bool onGestureRecord (int64_t gestureId) override;
};
