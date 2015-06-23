#include "UIGestureWindow.h"
#include "common/FileSystem.h"
#include "common/Log.h"
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
		Log::info(LOG_CLIENT, "Could not start gesture recording");
	} else {
		Log::info(LOG_CLIENT, "Started gesture recording");
	}
}

bool UIGestureWindow::onGestureRecord (int64_t gestureId)
{
	Log::info(LOG_CLIENT, "Save gestures");
	const bool retVal = UIWindow::onGestureRecord(gestureId);
	const std::string path = FS.getAbsoluteWritePath() + "gesture-" + string::toString(gestureId);
	SDL_RWops* rwops = SDL_RWFromFile(path.c_str(), "wb");
	if (rwops) {
		Log::info(LOG_CLIENT, "Save gestures to %s", path.c_str());
		if (SDL_SaveDollarTemplate(gestureId, rwops) <= 0) {
			Log::info(LOG_CLIENT, "Failed to save gestures to %s", path.c_str());
		}
		SDL_RWclose(rwops);
	} else {
		Log::info(LOG_CLIENT, "Failed to save gestures to %s", path.c_str());
	}
	UI::get().pop();
	return retVal;
}
