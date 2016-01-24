#include "UICaveExpressSettingsWindow.h"
#include "ui/windows/modeselection/ModeSetListener.h"
#include "campaign/CampaignManager.h"
#include "ui/nodes/UINodeButton.h"

namespace caveexpress {

UICaveExpressSettingsWindow::UICaveExpressSettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager) :
	UISettingsWindow(frontend, serviceProvider), _campaignManager(campaignManager), _gameModeNormal(nullptr), _gameModeHard(nullptr)
{
	init();
}

void UICaveExpressSettingsWindow::addSections()
{
	UISettingsWindow::addSections();

	addSection(_settingsGame, tr("Game mode"), "gamemode",
			tr("Normal"), new ModeSetListener("easy", _campaignManager),
			tr("Hard"), new ModeSetListener("hard", _campaignManager));
}

void UICaveExpressSettingsWindow::update (uint32_t time)
{
	if (!_gameModeNormal) {
		_gameModeNormal = (UINodeButton*)getNode("gamemode_1");
		_gameModeHard = (UINodeButton*)getNode("gamemode_2");
	}

	UISettingsWindow::update(time);
	UIWINDOW_SETTINGS_COLOR(Config.isModeEasy(), _gameModeNormal, _gameModeHard)
}

}
