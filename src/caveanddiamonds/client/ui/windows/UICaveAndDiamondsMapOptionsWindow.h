#pragma once

#include "engine/client/ui/windows/UIMapOptionsWindow.h"

// forward decl
class ServiceProvider;
class UINodeButtonImage;

class UICaveAndDiamondsMapOptionsWindow: public UIMapOptionsWindow {
private:
	UINodeButtonImage *_solve;
public:
	UICaveAndDiamondsMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void onActive () override;
};
