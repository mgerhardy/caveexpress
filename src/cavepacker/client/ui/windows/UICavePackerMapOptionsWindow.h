#pragma once

#include "ui/windows/UIMapOptionsWindow.h"

// forward decl
class ServiceProvider;
class UINodeButtonImage;

namespace cavepacker {

class UICavePackerMapOptionsWindow: public UIMapOptionsWindow {
private:
	UINodeButtonImage *_solve;
public:
	UICavePackerMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void onActive () override;
};

}
