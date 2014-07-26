#include "UINodeMapOnScreenCursorControl.h"
#include "engine/client/ui/windows/UIWindow.h"
#include "engine/client/ui/nodes/IUINodeMap.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"
#include "engine/client/ClientMap.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Logger.h"
#include "engine/common/Commands.h"

UINodeMapOnScreenCursorControl::UINodeMapOnScreenCursorControl (IFrontend *frontend, IUINodeMap *mapNode) :
		UINode(frontend), _map(mapNode->getMap())
{
	setStandardPadding();

	UINodeButton *left = new UINodeButton(frontend);
	left->setImage("icon-cursor-left");
	left->setOnActivate(CMD_MOVE_LEFT);
	left->setAlignment(NODE_ALIGN_LEFT | NODE_ALIGN_BOTTOM);
	add(left);
	UINodeButton *right = new UINodeButton(frontend);
	right->setImage("icon-cursor-right");
	right->setOnActivate(CMD_MOVE_RIGHT);
	right->putRight(left);
	add(right);

	UINodeButton *up = new UINodeButton(frontend);
	up->setImage("icon-cursor-up");
	up->setOnActivate(CMD_MOVE_UP);
	up->setAlignment(NODE_ALIGN_RIGHT | NODE_ALIGN_BOTTOM);
	add(up);
	UINodeButton *down = new UINodeButton(frontend);
	down->setImage("icon-cursor-down");
	down->setOnActivate(CMD_MOVE_DOWN);
	down->putLeft(up);
	add(down);
}

UINodeMapOnScreenCursorControl::~UINodeMapOnScreenCursorControl ()
{
}

bool UINodeMapOnScreenCursorControl::isActive () const
{
	return _map.isStarted() && !_map.isPause();
}
