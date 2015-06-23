#include "UINodeMapOnScreenCursorControl.h"
#include "ui/windows/UIWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "client/ClientMap.h"
#include "common/ConfigManager.h"
#include "common/Log.h"
#include "common/Commands.h"

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

	UINodeButton *down = new UINodeButton(frontend);
	down->setImage("icon-cursor-down");
	down->setOnActivate(CMD_MOVE_DOWN);
	down->setAlignment(NODE_ALIGN_RIGHT | NODE_ALIGN_BOTTOM);
	add(down);
	UINodeButton *up = new UINodeButton(frontend);
	up->setImage("icon-cursor-up");
	up->setOnActivate(CMD_MOVE_UP);
	up->putAbove(down);
	add(up);
}

UINodeMapOnScreenCursorControl::~UINodeMapOnScreenCursorControl ()
{
}

bool UINodeMapOnScreenCursorControl::isActive () const
{
	return _map.isStarted() && !_map.isPause();
}
