#include "UIMapFinishedWindow.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeMainButton.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/UINodeStar.h"
#include "ui/windows/main/ReplayNodeListener.h"
#include "ui/windows/main/ContinuePlayNodeListener.h"
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

	const float gapBack = std::max(0.01f, getScreenPadding());

	UINodeMainButton *continueCampaign = new UINodeMainButton(frontend, tr("Continue"));
	continueCampaign->alignTo(background, NODE_ALIGN_BOTTOM | NODE_ALIGN_RIGHT, gapBack);
	continueCampaign->addListener(UINodeListenerPtr(new ContinuePlayNodeListener(campaignManager, serviceProvider)));
	add(continueCampaign);

	_replayCampaign = new UINodeMainButton(frontend, tr("Retry"));
	_replayCampaign->alignTo(background, NODE_ALIGN_BOTTOM | NODE_ALIGN_CENTER, gapBack);
	_replayCampaign->addListener(UINodeListenerPtr(new ReplayNodeListener(campaignManager)));
	add(_replayCampaign);

	UINodePoint *points = new UINodePoint(frontend);
	points->setFont(LARGE_FONT);
	points->setId(UINODE_FINISHEDPOINTS);
	points->alignTo(background, NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE, 0.1f);
	add(points);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

bool UIMapFinishedWindow::onPush ()
{
	_replayCampaign->setVisible(!_serviceProvider.getNetwork().isMultiplayer());
	const int launchCount = Config.getConfigVar("mapfinishedcounter")->getIntValue();
	const int fullscreenAds = launchCount % 3;
	if (fullscreenAds == 1) {
		showFullscreenAds();
	}
	return UIWindow::onPush();
}
