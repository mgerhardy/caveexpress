#pragma once

#include "ui/windows/UIWindow.h"

namespace cavepacker {

class UIMainWindow: public UIWindow {
public:
	explicit UIMainWindow (IFrontend *frontend);
};

}
