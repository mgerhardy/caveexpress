#pragma once

#include "UIWindow.h"
#include "client/round/RoundController.h"

class IGameStatePersister;
class IProgressCallback;

class UIMapWindow: public UIWindow {
private:
	RoundController& _controller;
public:
	UIMapWindow (IFrontend *frontend, RoundController& roundController);
	virtual ~UIMapWindow ();
};
