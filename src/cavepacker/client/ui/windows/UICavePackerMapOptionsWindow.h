#pragma once

#include "ui/windows/UIMapOptionsWindow.h"

// forward decl
class ServiceProvider;
class UINodeMainButton;

namespace cavepacker {

class UICavePackerMapOptionsWindow: public UIMapOptionsWindow {
private:
	UINode *_solve;
	UINodeMainButton* _skipButton;
public:
	UICavePackerMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void onActive () override;
};

}
