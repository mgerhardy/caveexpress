#include "UIMainWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/common/Commands.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Version.h"
#include "engine/client/ui/layouts/UIVBoxLayout.h"
#include "engine/client/ui/windows/listener/OpenWindowListener.h"

UIMainWindow::UIMainWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_MAIN, frontend, WINDOW_FLAG_ROOT)
{
	UINode* background = new UINode(frontend);
	background->setImage("bg");
	background->setSize(1.0f, 1.0f);
	background->setPos(0.0f, 0.0f);
	add(background);

	const float padding = 10.0f / static_cast<float>(_frontend->getHeight());
	UINode *panel = new UINode(_frontend, "panelMain");
	UIVBoxLayout *layout = new UIVBoxLayout(padding, true);
	panel->setLayout(layout);
	panel->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	panel->setPadding(padding);

	/* @todo launch website when logo clicked? or convert to UINode if just image */
	/* @todo plug in logo */
	UINodeButton *logo = new UINodeButton(frontend, tr("Logo"));
	logo->addListener(UINodeListenerPtr(new OpenURLListener(frontend, "https://someurl.com")));
	panel->add(logo);

	UINodeLabel* topScoreLabel = new UINodeLabel(frontend, "TOP SCORE", getFont(""));
	panel->add(topScoreLabel);

	// @todo put in top score
	UINodeLabel* topScore = new UINodeLabel(frontend, "0", getFont(""));
	panel->add(topScore);

	UINodeButton* startButton = new UINodeButton(frontend, "Start");
	startButton->setBackgroundColor(colorBrightGreen);
	startButton->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_MAP)));
	panel->add(startButton);

	UINodeButton* settingsButton = new UINodeButton(frontend, "Settings");
	settingsButton->setBackgroundColor(colorBrightGreen);
	settingsButton->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_SETTINGS)));
	panel->add(settingsButton);

	UINodeButton* quitButton = new UINodeButton(frontend, "Quit");
	quitButton->setBackgroundColor(colorRed);
	quitButton->setOnActivate(CMD_QUIT);
	panel->add(quitButton);

	add(panel);
}

UIMainWindow::~UIMainWindow ()
{
}
