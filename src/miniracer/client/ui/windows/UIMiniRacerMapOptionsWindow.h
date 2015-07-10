#pragma once

#include "ui/windows/UIMapOptionsWindow.h"

// forward decl
class ServiceProvider;
class UINodeButtonImage;

namespace miniracer {

class UIMiniRacerMapOptionsWindow: public UIMapOptionsWindow {
private:
	UINodeButtonImage *_solve;
public:
	UIMiniRacerMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void onActive () override;
};

}
