#include "UIRoundResultWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/windows/OpenWindowListener.h"
#include "geophoto/client/ui/nodes/UINodeLocationSelector.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeLabel.h"

UIRoundResultWindow::UIRoundResultWindow(IFrontend *frontend,
		const GameRound& gameRound) :
		UIWindow(UI_WINDOW_ROUNDRESULT, frontend, WINDOW_FLAG_MODAL), _gameRound(gameRound) {
	UINode* background = new UINode(frontend);
	background->setImage("bg");
	background->setSize(1.0f, 1.0f);
	background->setPos(0.0f, 0.0f);
	add(background);

	_totalScore = new UINodeLabel(frontend, "0", getFont(""));
	_totalScore->setStandardSpacing();
	_totalScore->setPos(_totalScore->getSpacingX(), _totalScore->getSpacingY());
	add(_totalScore);

	UINodeLabel* labelScore = new UINodeLabel(frontend, "SCORE", getFont(""));
	labelScore->setStandardSpacing();
	labelScore->putUnder(_totalScore, labelScore->getSpacingIntervalY(0.2f));
	add(labelScore);

	// @todo put in top score
	_totalTopScore = new UINodeLabel(frontend, "0", getFont(""));
	_totalTopScore->setStandardSpacing();
	_totalTopScore->putUnder(labelScore, _totalTopScore->getSpacingY());
	add(_totalTopScore);

	UINodeLabel* labelTopScore = new UINodeLabel(frontend, "TOP SCORE", getFont(""));
	labelTopScore->setStandardSpacing();
	labelTopScore->putUnder(_totalTopScore, labelTopScore->getSpacingIntervalY(0.2f));
	add(labelTopScore);

	UINodeButton* viewResults = new UINodeButton(frontend, "View Results");
	viewResults->setBackgroundColor(colorYellow);
	viewResults->setStandardSpacing();
	viewResults->setSize(viewResults->getSpacingIntervalX(15), viewResults->getSpacingIntervalY(3));
	viewResults->setPos(viewResults->getSpacingX(), 1.0f - viewResults->getSpacingIntervalY(4));
	viewResults->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_VIEWRESULT)));
	add(viewResults);

	UINodeButton* shareResults = new UINodeButton(frontend, "Share Results");
	shareResults->setBackgroundColor(colorRed);
	shareResults->setStandardSpacing();
	shareResults->setSize(shareResults->getSpacingIntervalX(15), shareResults->getSpacingIntervalY(3));
	shareResults->putAbove(viewResults, shareResults->getSpacingY());
//	@todo open window to post facebook or twitter
//	shareResults->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_VIEWRESULT)));
	add(shareResults);

	/* @todo launch website when logo clicked? or convert to UINode if just image */
	/* @todo plug in logo */
	UINodeButton* logo = new UINodeButton(frontend, "Logo");
	logo->setBackgroundColor(colorDark);
	logo->setStandardSpacing();
	logo->setSize(logo->getSpacingIntervalX(14), logo->getSpacingIntervalY(10.5));
	logo->setPos(logo->getSpacingIntervalX(17),logo->getSpacingY());
	logo->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "http://someurl.com")));
	add(logo);

	UINodeButton* playAgain = new UINodeButton(frontend, "Play Again");
	playAgain->setBackgroundColor(colorBrightGreen);
	playAgain->setStandardSpacing();
	playAgain->setSize(playAgain->getSpacingIntervalX(14), logo->getSpacingIntervalY(3));
	playAgain->setPos(playAgain->getSpacingIntervalX(17), 1.0f - logo->getSpacingIntervalY(4));
	playAgain->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_MAP)));
	add(playAgain);
}

UIRoundResultWindow::~UIRoundResultWindow() {
}

bool UIRoundResultWindow::onPush() {
	const bool val = UIWindow::onPush();

	const Locations& locations = _gameRound.getLocations();
	int roundScore = 0;
	for (Locations::const_iterator i = locations.begin(); i != locations.end(); ++i) {
		roundScore += i->score;
	}

	_totalScore->setLabel(string::toString(roundScore));

	// _totalTopScore->setLabel("");

	return val;
}
