#include "UIModeSelectionWindow.h"
#include "client/ui/nodes/UINodeMainButton.h"
#include "client/ui/layouts/UIVBoxLayout.h"
#include "common/ConfigManager.h"
#include "client/ui/nodes/UINodeBackground.h"
#include "modeselection/ModeSetListener.h"

UIModeSelectionWindow::UIModeSelectionWindow (IFrontend *frontend, CampaignManager& campaignManager) :
		UIWindow(UI_WINDOW_MODE_SELECTION, frontend, WINDOW_FLAG_MODAL)
{
	add(new UINodeBackground(frontend, tr("Select game mode")));

	UINode *panel = new UINode(_frontend, "mode-select-panel");
	UIVBoxLayout *layout = new UIVBoxLayout(0.05f, true);
	panel->setLayout(layout);
	panel->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);

	_easyMode = new UINodeMainButton(frontend, tr("Normal"));
	_easyMode->addListener(UINodeListenerPtr(new ModeSetListener("easy", campaignManager)));
	panel->add(_easyMode);

	_hardMode = new UINodeMainButton(frontend, tr("Hard"));
	_hardMode->addListener(UINodeListenerPtr(new ModeSetListener("hard", campaignManager)));
	panel->add(_hardMode);

	add(panel);
}

bool UIModeSelectionWindow::onPop ()
{
	if (!Config.isModeSelected())
		return false;
	return UIWindow::onPop();
}
