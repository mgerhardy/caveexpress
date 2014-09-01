#include "UIMainWindow.h"
#include "client/ui/UI.h"
#include "client/ui/windows/OpenWindowListener.h"
#include "client/ui/nodes/UINodeButton.h"
#include "client/ui/nodes/UINodePanel.h"
#include "client/ui/nodes/UINodeLabel.h"
#include "shared/constants/Commands.h"
#include "shared/ConfigManager.h"
#include "shared/Version.h"

UIMainWindow::UIMainWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_MAIN, frontend, WINDOW_FLAG_ROOT)
{
	UINode* background = new UINode(frontend);
	background->setImage("bg");
	background->setSize(1.0f, 1.0f);
	background->setPos(0.0f, 0.0f);
	add(background);

	/* @todo launch website when logo clicked? or convert to UINode if just image */
	/* @todo plug in logo */
	UINodeButton* logo = new UINodeButton(frontend, "Logo");
	logo->setBackgroundColor(colorDark);
	logo->setStandardSpacing();
	logo->setSize(logo->getSpacingIntervalX(14), logo->getSpacingIntervalY(10.5));
	logo->setPos(logo->getSpacingX(),logo->getSpacingY());
	logo->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "http://someurl.com")));
	add(logo);

	UINodeLabel* topScoreLabel = new UINodeLabel(frontend, "TOP SCORE", getFont(""));
	topScoreLabel->setStandardSpacing();
	topScoreLabel->putUnder(logo, topScoreLabel->getSpacingY());
	add(topScoreLabel);

	// @todo put in top score
	UINodeLabel* topScore = new UINodeLabel(frontend, "0", getFont(""));
	topScore->setStandardSpacing();
	topScore->putUnder(logo, topScore->getSpacingY());
	topScore->setPos(topScore->getLeft() + logo->getWidth() - topScore->getWidth(), topScore->getTop());
	add(topScore);

	UINodeButton* startButton = new UINodeButton(frontend, "Start");
	startButton->setBackgroundColor(colorBrightGreen);
	startButton->setStandardSpacing();
	startButton->setSize(startButton->getSpacingIntervalX(15), startButton->getSpacingIntervalY(3));
	startButton->setPos(startButton->getSpacingIntervalX(16), startButton->getSpacingY());
	startButton->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_MAP)));
	add(startButton);

	UINodeButton* settingsButton = new UINodeButton(frontend, "Settings");
	settingsButton->setBackgroundColor(colorBrightGreen);
	settingsButton->setStandardSpacing();
	settingsButton->setSize(settingsButton->getSpacingIntervalX(15), settingsButton->getSpacingIntervalY(3));
	settingsButton->putUnder(startButton, settingsButton->getSpacingY());
	settingsButton->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_SETTINGS)));
	add(settingsButton);

	UINodeButton* quitButton = new UINodeButton(frontend, "Quit");
	quitButton->setBackgroundColor(colorRed);
	quitButton->setStandardSpacing();
	quitButton->setSize(quitButton->getSpacingIntervalX(15), quitButton->getSpacingIntervalY(3));
	quitButton->setPos(quitButton->getSpacingIntervalX(16), 1.0f - quitButton->getSpacingIntervalY(4));
	quitButton->setOnActivate(CMD_QUIT);
	add(quitButton);
}

UIMainWindow::~UIMainWindow ()
{
}
