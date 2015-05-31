#include "UICaveExpressSettingsWindow.h"
#include "client/ui/windows/modeselection/ModeSetListener.h"
#include "common/campaign/CampaignManager.h"

UICaveExpressSettingsWindow::UICaveExpressSettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager) :
	UISettingsWindow(frontend, serviceProvider), _campaignManager(campaignManager)
{
	init();
}

UINode* UICaveExpressSettingsWindow::addSections()
{
	UINode* last = UISettingsWindow::addSections();

	last = addSection(last, nullptr, tr("Game mode"),
			tr("Normal"), new ModeSetListener("easy", _campaignManager),
			tr("Hard"), new ModeSetListener("hard", _campaignManager));

	return last;
}
