#include "UISettingsWindow.h"
#include "geophoto/client/ui/windows/settings/SetDifficultyNodeListener.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/common/Commands.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Version.h"
#include "engine/common/String.h"

UISettingsWindow::UISettingsWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_SETTINGS, frontend, WINDOW_FLAG_MODAL)
{
	UINode* background = new UINode(frontend);
	background->setImage("bg");
	background->setSize(1.0f, 1.0f);
	background->setPos(0.0f, 0.0f);
	add(background);

	UINodePanel *panelDifficulty = new UINodePanel(frontend);
	panelDifficulty->setPos(0.025f, 0.05f);
	panelDifficulty->setSpacing(0.025f, 0.05f);
	panelDifficulty->setLayout(LAYOUT_GRID_X);

	std::vector<UINodeButton*> difficultyButtons;

	const std::string& currentDifficulty = Config.getDifficulty();
	const char *difficulties[] = {"Easy", "Normal", "Hard"};
	for (int i = 0; i < lengthof(difficulties); ++i) {
		UINodeButton* difficultyButton = new UINodeButton(frontend, difficulties[i]);
		difficultyButton->setSize(0.3f, 0.1f);
		panelDifficulty->add(difficultyButton);
		difficultyButtons.push_back(difficultyButton);
		const std::string difficultyValue = string::toLower(difficulties[i]);
		if (difficultyValue == currentDifficulty) {
			difficultyButton->setBackgroundColor(colorBrightGreen);
		} else {
			difficultyButton->setImage("bg");
		}
	}

	for (std::vector<UINodeButton*>::iterator i = difficultyButtons.begin(); i != difficultyButtons.end(); ++i) {
		(*i)->addListener(UINodeListenerPtr(new SetDifficultyNodeListener(*i, difficultyButtons)));
	}

	add(panelDifficulty);

	UINodeBackButton* backButton = new UINodeBackButton(frontend);
	backButton->setSize(0.475f, 0.1f);
	backButton->setBackgroundColor(colorRed);
	backButton->alignTo(background, NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT, 0.01f);
	add(backButton);
}

UISettingsWindow::~UISettingsWindow ()
{
}
