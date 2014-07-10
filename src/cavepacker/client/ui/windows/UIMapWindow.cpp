#include "UIMapWindow.h"
#include "engine/client/ui/UI.h"

UIMapWindow::UIMapWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_MAP, frontend, WINDOW_FLAG_FULLSCREEN)
{
}
