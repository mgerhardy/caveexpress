#include "UIGestureWindow.h"
#include "engine/common/FileSystem.h"

#include <SDL.h>

UIGestureWindow::UIGestureWindow(IFrontend *frontend) :
		UIWindow("gesture", frontend, WINDOW_FLAG_MODAL) {
}

void UIGestureWindow::onActive() {
	UIWindow::onActive();

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
	const URI uriLocal("file://" + FS.getAbsoluteWritePath() + "gesture-" + string::toString(gestureId));
	std::string path;
	const FilePtr& f = FS.getFile(uriLocal);
	SDL_RWops* rwops = SDL_RWFromFile(f->getURI().getPath().c_str(), "wb");
	if (rwops) {
		info(LOG_CLIENT, "Save gestures to " + f->getURI().getPath());
		if (SDL_SaveAllDollarTemplates(rwops) <= 0) {
			info(LOG_CLIENT, "Failed to save gestures to " + f->getURI().getPath());
		}
		SDL_RWclose(rwops);
	} else {
		info(LOG_CLIENT, "Failed to save gestures to " + f->getURI().getPath());
	}
	return retVal;
}
