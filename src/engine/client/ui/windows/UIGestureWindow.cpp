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
	const URI uriLocal("file://" + FS.getAbsoluteWritePath() + "gesture");
	std::string path;
	const FilePtr& f = FS.getFile(uriLocal);
	SDL_RWops* rwops = SDL_RWFromFile(f->getURI().getPath().c_str(), "wb");
	SDL_SaveAllDollarTemplates(rwops);
	SDL_RWclose(rwops);
	return retVal;
}
