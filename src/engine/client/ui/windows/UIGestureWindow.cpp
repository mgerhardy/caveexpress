#include "UIGestureWindow.h"

#include <SDL.h>

UIGestureWindow::UIGestureWindow(IFrontend *frontend) :
		UIWindow("gesture", frontend, WINDOW_FLAG_MODAL) {
}

void UIGestureWindow::onActive() {
	UIWindow::onActive();

	if (!SDL_RecordGesture(-1)) {
		return;
	}
}

bool UIGestureWindow::onGestureRecord (int64_t gestureId)
{
	const bool retVal = UIWindow::onGestureRecord(gestureId);
	info(LOG_CLIENT, "TODO: save gesture");
	return retVal;
}

bool UIGestureWindow::onGesture (int64_t gestureId)
{
	const bool retVal = UIWindow::onGesture(gestureId);
	info(LOG_CLIENT, String::format("detected gesture %i", gestureId));
	return retVal;
}

bool UIGestureWindow::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
	const bool retVal = UIWindow::onFingerMotion(finger, x, y, dx, dy);
	// TODO: record
	return retVal;
}
