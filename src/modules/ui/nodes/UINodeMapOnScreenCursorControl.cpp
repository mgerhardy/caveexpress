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
	setId("cursor-control");

	_left = new UINodeButton(frontend);
	_left->setImage("icon-cursor-left");
	_left->setOnActivate(CMD_MOVE_LEFT);
	_left->setTriggerTime(500u);
	_left->setAlignment(NODE_ALIGN_LEFT | NODE_ALIGN_BOTTOM);
	add(_left);
	_right = new UINodeButton(frontend);
	_right->setImage("icon-cursor-right");
	_right->setOnActivate(CMD_MOVE_RIGHT);
	_right->setTriggerTime(500u);
	_right->putRight(_left);
	add(_right);

	_down = new UINodeButton(frontend);
	_down->setImage("icon-cursor-down");
	_down->setOnActivate(CMD_MOVE_DOWN);
	_down->setAlignment(NODE_ALIGN_RIGHT | NODE_ALIGN_BOTTOM);
	_down->setTriggerTime(500u);
	add(_down);
	_up = new UINodeButton(frontend);
	_up->setImage("icon-cursor-up");
	_up->setOnActivate(CMD_MOVE_UP);
	_up->setTriggerTime(500u);
	_up->putAbove(_down);
	add(_up);
}

UINodeMapOnScreenCursorControl::~UINodeMapOnScreenCursorControl ()
{
}

bool UINodeMapOnScreenCursorControl::isActive () const
{
	return _map.isStarted() && !_map.isPause();
}
