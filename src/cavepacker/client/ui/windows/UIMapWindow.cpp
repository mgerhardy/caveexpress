#include "UIMapWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodePoint.h"
#include "engine/common/Commands.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map) :
	UIWindow(UI_WINDOW_MAP, frontend, WINDOW_FLAG_MODAL | WINDOW_FLAG_FULLSCREEN), _serviceProvider(serviceProvider)
{
	const float screenPadding = getScreenPadding();
	setPadding(screenPadding);
	_playClickSound = false;
	_musicFile = "music-2";
	_onPop = CMD_CL_DISCONNECT;
	_nodeMap = new UINodeMap(frontend, _serviceProvider, campaignManager, 0, 0, _frontend->getWidth(), _frontend->getHeight(), map);
	add(_nodeMap);

	_panel = new UINode(frontend);
	UIHBoxLayout* layout = new UIHBoxLayout();
	layout->setSpacing(0.02f);
	_panel->setLayout(layout);
	_panel->setStandardPadding();
	_panel->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_CENTER);

	_points = new UINodePoint(frontend, 150);
	_points->setLabel("00000");
	_points->setId("POINTS");
	_panel->add(_points);
}
