#include "UIMapWindow.h"
#include "client/ui/UI.h"
#include "client/ui/nodes/UINodeButton.h"
#include "client/ui/nodes/UINodePanel.h"
#include "client/ui/nodes/UINodeBar.h"
#include "client/ui/nodes/UINodeLabel.h"
#include "client/ui/nodes/UINodeMap.h"

UIMapWindow::UIMapWindow (IFrontend *frontend, RoundController& controller) :
		UIWindow(UI_WINDOW_MAP, frontend, WINDOW_FLAG_FULLSCREEN), _controller(controller)
{
	UINodeBar *timeBar = new UINodeBar(frontend);
	timeBar->setSize(1.0f, 20.0f / static_cast<float>(_frontend->getHeight()));
	timeBar->setPos(0.0f, 1.0f - timeBar->getHeight());
	timeBar->setVisible(false);
	timeBar->setBorder(false);

	UINodeLabel *timeNode = new UINodeLabel(frontend, "0", getFont(""));
	timeNode->setPos(0.0f, 0.99f - timeNode->getHeight() - timeBar->getHeight());
	timeNode->setVisible(false);

	UINodeMap *map = new UINodeMap(frontend, _controller);
	map->setStandardSpacing();
	map->setSize(1.0f, map->getSpacingIntervalY(16));
	map->setAlignment(NODE_ALIGN_MIDDLE);

	_controller.init(map, timeBar, timeNode);

	add(map);
	add(timeBar);
	add(timeNode);
}

UIMapWindow::~UIMapWindow ()
{
}
