#include "UIGestureWindow.h"
#include "engine/common/FileSystem.h"

#include <SDL.h>

UIGestureWindow::UIGestureWindow(IFrontend *frontend) :
		UIWindow("gesture", frontend, WINDOW_FLAG_MODAL) {
}

void UIGestureWindow::onActive() {
	UIWindow::onActive();

	const FilePtr& f = FS.getFile("gesture");
	SDL_RWops* rwops = SDL_RWFromFile(f->getURI().getPath().c_str(), "rb");
	if (SDL_LoadDollarTemplates(-1, rwops) == 0) {
		info(LOG_CLIENT, "Could not load " + f->getURI().getPath());
	}

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

bool UIGestureWindow::onGesture (int64_t gestureId)
{
	const bool retVal = UIWindow::onGesture(gestureId);
	info(LOG_CLIENT, String::format("detected gesture %i", gestureId));
	return retVal;
}
