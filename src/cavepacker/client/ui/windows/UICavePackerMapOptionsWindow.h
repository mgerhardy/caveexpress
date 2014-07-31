#pragma once

#include "engine/client/ui/windows/UIMapOptionsWindow.h"

// forward decl
class ServiceProvider;

class UICavePackerMapOptionsWindow: public UIMapOptionsWindow {
public:
	UICavePackerMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
};
