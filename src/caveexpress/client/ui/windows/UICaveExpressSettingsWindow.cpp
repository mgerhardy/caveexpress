#include "UICaveExpressSettingsWindow.h"
#include "ui/windows/modeselection/ModeSetListener.h"
#include "campaign/CampaignManager.h"

namespace caveexpress {

UICaveExpressSettingsWindow::UICaveExpressSettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager) :
	UISettingsWindow(frontend, serviceProvider), _campaignManager(campaignManager)
{
	init();
}

UINode* UICaveExpressSettingsWindow::addSections()
{
	UINode* last = UISettingsWindow::addSections();

	last = addSection(last, nullptr, tr("Game mode"), "game-mode",
			tr("Normal"), new ModeSetListener("easy", _campaignManager),
			tr("Hard"), new ModeSetListener("hard", _campaignManager));

	return last;
}

}
