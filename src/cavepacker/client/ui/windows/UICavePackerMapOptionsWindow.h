#pragma once

#include "ui/windows/UIMapOptionsWindow.h"

// forward decl
class ServiceProvider;
class UINode;

namespace cavepacker {

class UICavePackerMapOptionsWindow: public UIMapOptionsWindow {
private:
	UINode *_solve;
public:
	UICavePackerMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void onActive () override;
};

}
