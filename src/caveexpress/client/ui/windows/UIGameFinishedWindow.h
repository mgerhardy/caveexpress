#pragma once

#include "ui/windows/UIWindow.h"

namespace caveexpress {

class UIGameFinishedWindow: public UIWindow {
public:
	explicit UIGameFinishedWindow (IFrontend *frontend);
};

}
