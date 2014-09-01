#pragma once

#include "UIWindow.h"

class GameRound;
class UINodeLocationSelector;

class UIViewResultWindow: public UIWindow {
private:
	const GameRound& _gameRound;
	UINodeLocationSelector *_selector;
public:
	UIViewResultWindow (IFrontend *frontend, const GameRound& gameRound);
	virtual ~UIViewResultWindow ();

	bool onPush () override;
};
