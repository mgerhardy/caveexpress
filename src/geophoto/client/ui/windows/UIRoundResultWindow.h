#pragma once

#include "engine/client/ui/windows/UIWindow.h"

class GameRound;
class UINodeLabel;

class UIRoundResultWindow: public UIWindow {
private:
	const GameRound& _gameRound;

	UINodeLabel* _totalScore;
	UINodeLabel* _totalTopScore;
public:
	UIRoundResultWindow (IFrontend *frontend, const GameRound& gameRound);
	virtual ~UIRoundResultWindow ();

	bool onPush () override;
};
