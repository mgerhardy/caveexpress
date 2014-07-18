#include "UICampaignWindow.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeButtonImage.h"
#include "engine/client/ui/windows/listener/SelectorPageListener.h"
#include "engine/client/ui/nodes/UINodeMainButton.h"
#include "engine/common/ServiceProvider.h"
#include "engine/client/ui/nodes/UINodeCampaignSelector.h"
#include "engine/client/ui/nodes/UINodeBackground.h"
#include "engine/client/ui/windows/main/ContinuePlayNodeListener.h"

UICampaignWindow::UICampaignWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager) :
		UIWindow(UI_WINDOW_CAMPAIGN, frontend), _campaignManager(campaignManager)
{
	UINodeBackground *background = new UINodeBackground(frontend, tr("Campaigns"));
	add(background);

	const float gap = 0.001f;

	_buttonLeft = new UINodeButtonImage(frontend, "icon-scroll-page-left");

	int cols = 6;
	int rows = 4;
	_campaign = new UINodeCampaignSelector(frontend, _campaignManager, cols, rows);
	add(_campaign);

	_buttonLeft->setPos(_campaign->getLeft() - _buttonLeft->getWidth() - gap, _campaign->getTop());
	_buttonLeft->addListener(UINodeListenerPtr(new SelectorPageListener<CampaignPtr>(_campaign, false)));
	add(_buttonLeft);

	_buttonRight = new UINodeButtonImage(frontend, "icon-scroll-page-right");
	_buttonRight->setPos(_campaign->getRight() + gap, _campaign->getTop());
	_buttonRight->addListener(UINodeListenerPtr(new SelectorPageListener<CampaignPtr>(_campaign, true)));
	add(_buttonRight);

	_continuePlay = new UINodeMainButton(_frontend, tr("Continue"));
	_continuePlay->addListener(UINodeListenerPtr(new ContinuePlayNodeListener(_campaignManager, serviceProvider)));
	_continuePlay->putUnderRight(_campaign);
	add(_continuePlay);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

UICampaignWindow::~UICampaignWindow ()
{
}

void UICampaignWindow::onActive ()
{
	UIWindow::onActive();
	const bool isCompleted = _campaignManager.isCompleted();
	_continuePlay->setVisible(!isCompleted);
}

void UICampaignWindow::update (uint32_t deltaTime)
{
	UIWindow::update(deltaTime);
	_buttonRight->setVisible(_campaign->hasMoreEntries());
	_buttonLeft->setVisible(_campaign->hasLessEntries());
}
