#pragma once

#include "ui/windows/UIWindow.h"

namespace miniracer {

class UIMainWindow: public UIWindow {
public:
	explicit UIMainWindow (IFrontend *frontend);
};

}
