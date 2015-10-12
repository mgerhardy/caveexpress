#include "UIMapFinishedWindow.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/UINodeContinuePlay.h"
#include "ui/nodes/UINodeStar.h"
#include "ui/windows/main/ReplayNodeListener.h"
#include "common/ConfigManager.h"
#include <string>

UIMapFinishedWindow::UIMapFinishedWindow (IFrontend *frontend, CampaignManager& campaignManager, ServiceProvider& serviceProvider, const SoundType& soundType) :
		UIWindow(UI_WINDOW_MAPFINISHED, frontend), _serviceProvider(serviceProvider)
{
	setInactiveAfterPush();

	UINodeBackground *background = new UINodeBackground(frontend, "", false);
	if (System.hasTouch() && !wantBackButton())
		background->setOnActivate(CMD_UI_POP);
	add(background);

	UINode *stars = new UINode(frontend);
	stars->setPadding(getScreenPadding());
	stars->setAlignment(NODE_ALIGN_CENTER);
	stars->setLayout(new UIHBoxLayout());
	for (int i = 0; i < 3; ++i) {
		UINodeStar *star = new UINodeStar(frontend, i, soundType);
		stars->add(star);
	}
	add(stars);

	UINode *bigButtons = new UINode(frontend);
	bigButtons->setLayout(new UIHBoxLayout());
	bigButtons->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);

	UINodeContinuePlay *continueCampaign = new UINodeContinuePlay(frontend, campaignManager, serviceProvider);
	bigButtons->add(continueCampaign);

	_replayCampaign = new UINodeButton(frontend);
	_replayCampaign->setImage("icon-reload");
	_replayCampaign->setSize(continueCampaign->getWidth(), continueCampaign->getHeight());
	_replayCampaign->addListener(UINodeListenerPtr(new ReplayNodeListener(campaignManager)));
	bigButtons->add(_replayCampaign);

	add(bigButtons);

	UINodePoint *points = new UINodePoint(frontend);
	points->setFont(LARGE_FONT);
	points->setId(UINODE_FINISHEDPOINTS);
	points->alignTo(bigButtons, NODE_ALIGN_CENTER, 0.1f);
	points->setPos(points->getX(), bigButtons->getBottom());
	add(points);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

bool UIMapFinishedWindow::onPush ()
{
	_replayCampaign->setVisible(!_serviceProvider.getNetwork().isMultiplayer());
	const int launchCount = Config.increaseCounter("mapfinishedcounter");
	const int fullscreenAds = launchCount % 3;
	if (fullscreenAds == 2) {
		showFullscreenAds();
	}
	return UIWindow::onPush();
}
