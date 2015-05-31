#include "UIGestureWindow.h"
#include "common/FileSystem.h"
#include "common/Logger.h"
#include "ui/UI.h"

#include <SDL.h>

UIGestureWindow::UIGestureWindow(IFrontend *frontend) :
		UIWindow("gesture", frontend, WINDOW_FLAG_MODAL) {
}

void UIGestureWindow::render (int x, int y) const {
	UIWindow::render(x, y);
	for (CoordsConstIter i = _coords.begin(); i != _coords.end(); ++i) {
		const vec2& v = *i;
		_frontend->renderRect(x + v.x, y + v.y, 4, 4, colorRed);
	}
}

bool UIGestureWindow::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) {
	const bool retVal = UIWindow::onFingerMotion(finger, x, y, dx, dy);
	_coords.push_back(vec2(x, y));
	return retVal;
}

void UIGestureWindow::onActive() {
	UIWindow::onActive();

	_coords.clear();

	if (!SDL_RecordGesture(-1)) {
		info(LOG_CLIENT, "Could not start gesture recording");
	} else {
		info(LOG_CLIENT, "Started gesture recording");
	}
}

bool UIGestureWindow::onGestureRecord (int64_t gestureId)
{
	info(LOG_CLIENT, "Save gestures");
	const bool retVal = UIWindow::onGestureRecord(gestureId);
	const std::string path = FS.getAbsoluteWritePath() + "gesture-" + string::toString(gestureId);
	SDL_RWops* rwops = SDL_RWFromFile(path.c_str(), "wb");
	if (rwops) {
		info(LOG_CLIENT, "Save gestures to " + path);
		if (SDL_SaveDollarTemplate(gestureId, rwops) <= 0) {
			info(LOG_CLIENT, "Failed to save gestures to " + path);
		}
		SDL_RWclose(rwops);
	} else {
		info(LOG_CLIENT, "Failed to save gestures to " + path);
	}
	UI::get().pop();
	return retVal;
}
