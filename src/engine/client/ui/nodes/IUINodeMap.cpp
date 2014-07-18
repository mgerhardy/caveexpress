#include "IUINodeMap.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/Commands.h"
#include "engine/common/EventHandler.h"
#include "engine/common/Direction.h"
#include "engine/client/commands/CmdConnect.h"
#include "engine/client/commands/CmdDisconnect.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/BitmapFont.h"
#include "engine/client/ui/windows/IUIMapWindow.h"
#include "engine/client/network/ChangeAnimationHandler.h"
#include "engine/client/network/MapRestartHandler.h"
#include "engine/client/network/PauseHandler.h"
#include "engine/client/network/UpdateEntityHandler.h"
#include "engine/client/network/RemoveEntityHandler.h"
#include "engine/client/network/SoundHandler.h"
#include "engine/client/network/BackToMainHandler.h"
#include "engine/client/network/RumbleHandler.h"
#include "engine/client/network/PlayerListHandler.h"
#include "engine/client/network/TextMessageHandler.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/common/ServiceProvider.h"
#include "engine/client/commands/CmdMove.h"
#include "engine/common/campaign/CampaignManager.h"
#include "engine/client/network/CloseMapHandler.h"
#include "engine/client/network/HudLoadMapHandler.h"
#include "engine/client/network/HudMapSettingsHandler.h"
#include "engine/client/network/InitWaitingMapHandler.h"
#include "engine/client/network/StartMapHandler.h"

IUINodeMap::IUINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, ClientMap& map) :
		UINode(frontend), _map(map), _campaignManager(campaignManager)
{
	Commands.registerCommand(CMD_CL_CONNECT, new CmdConnect(&_map, serviceProvider));
	Commands.registerCommand(CMD_CL_DISCONNECT, new CmdDisconnect(serviceProvider));
	Commands.registerCommand(CMD_MOVE_UP, new CmdMove(_map, DIRECTION_UP));
	Commands.registerCommand(CMD_MOVE_DOWN, new CmdMove(_map, DIRECTION_DOWN));
	Commands.registerCommand(CMD_MOVE_LEFT, new CmdMove(_map, DIRECTION_LEFT));
	Commands.registerCommand(CMD_MOVE_RIGHT, new CmdMove(_map, DIRECTION_RIGHT));

	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerClientHandler(protocol::PROTO_CHANGEANIMATION, new ChangeAnimationHandler(_map));
	r.registerClientHandler(protocol::PROTO_MAPRESTART, new MapRestartHandler(_map));
	r.registerClientHandler(protocol::PROTO_PAUSE, new PauseHandler(_map));
	r.registerClientHandler(protocol::PROTO_UPDATEENTITY, new UpdateEntityHandler(_map));
	r.registerClientHandler(protocol::PROTO_REMOVEENTITY, new RemoveEntityHandler(_map));
	r.registerClientHandler(protocol::PROTO_SOUND, new SoundHandler());
	r.registerClientHandler(protocol::PROTO_BACKTOMAIN, new BackToMainHandler());
	r.registerClientHandler(protocol::PROTO_RUMBLE, new RumbleHandler(_map));
	r.registerClientHandler(protocol::PROTO_PLAYERLIST, new PlayerListHandler(this));
	r.registerClientHandler(protocol::PROTO_MESSAGE, new TextMessageHandler(this));
	r.registerClientHandler(protocol::PROTO_CLOSEMAP, new CloseMapHandler(_map));
	r.registerClientHandler(protocol::PROTO_LOADMAP, new HudLoadMapHandler(_map, serviceProvider));
	r.registerClientHandler(protocol::PROTO_MAPSETTINGS, new HudMapSettingsHandler(_map));
	r.registerClientHandler(protocol::PROTO_INITWAITING, new InitWaitingMapHandler(serviceProvider));
	r.registerClientHandler(protocol::PROTO_STARTMAP, new StartMapHandler());

	_campaignManager.addListener(this);

	setBackgroundColor(colorNull);

	const float w = static_cast<float>(_frontend->getWidth());
	const float h = static_cast<float>(_frontend->getHeight());
	setPos(_map.getX() / w, _map.getY() / h);
	setSize(_map.getWidth() / w, _map.getHeight() / h);
}

IUINodeMap::~IUINodeMap ()
{
	Commands.removeCommand(CMD_CL_CONNECT);
	Commands.removeCommand(CMD_CL_DISCONNECT);
	Commands.removeCommand(CMD_MOVE_UP);
	Commands.removeCommand(CMD_MOVE_DOWN);
	Commands.removeCommand(CMD_MOVE_LEFT);
	Commands.removeCommand(CMD_MOVE_RIGHT);

	_campaignManager.removeListener(this);

	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.unregisterClientHandler(protocol::PROTO_CHANGEANIMATION);
	r.unregisterClientHandler(protocol::PROTO_MAPRESTART);
	r.unregisterClientHandler(protocol::PROTO_STARTMAP);
	r.unregisterClientHandler(protocol::PROTO_UPDATEENTITY);
	r.unregisterClientHandler(protocol::PROTO_REMOVEENTITY);
	r.unregisterClientHandler(protocol::PROTO_SOUND);
	r.unregisterClientHandler(protocol::PROTO_BACKTOMAIN);
	r.unregisterClientHandler(protocol::PROTO_RUMBLE);
	r.unregisterClientHandler(protocol::PROTO_PLAYERLIST);
	r.unregisterClientHandler(protocol::PROTO_MESSAGE);
	r.unregisterClientHandler(protocol::PROTO_CLOSEMAP);
	r.unregisterClientHandler(protocol::PROTO_LOADMAP);
	r.unregisterClientHandler(protocol::PROTO_MAPSETTINGS);
	r.unregisterClientHandler(protocol::PROTO_INITWAITING);
	r.unregisterClientHandler(protocol::PROTO_STARTMAP);
}

void IUINodeMap::setMapRect (int x, int y, int w, int h)
{
	_map.setSize(w, h);
	_map.setPos(x, y);
}

void IUINodeMap::update (uint32_t deltaTime)
{
	UINode::update(deltaTime);
	_map.update(deltaTime);
}

void IUINodeMap::render (int x, int y) const
{
	renderFilledRect(getRenderX(), getRenderY(), getRenderWidth(), getRenderHeight(), colorBlack);
	if (_map.isActive())
		_map.render(0, 0);
	UINode::render(0, 0);

	if (_map.isStarted())
		return;
	const BitmapFontPtr& font = getFont(HUGE_FONT);
	y += getRenderHeight() / 10;
	x += getRenderWidth() / 10;

	y += font->print("Players:", colorWhite, x, y) + font->getCharHeight();
	for (std::vector<std::string>::const_iterator i = _players.begin(); i != _players.end(); ++i) {
		y += font->print(*i, colorWhite, x + 10, y) + font->getCharHeight();
	}
}

bool IUINodeMap::onPush ()
{
	const bool val = UINode::onPush();
	if (!_campaignTextForNextPush.empty()) {
		displayText(_campaignTextForNextPush);
		_campaignTextForNextPush = "";
	}
	return val;
}

bool IUINodeMap::onPop ()
{
	_map.close();
	return UINode::onPop();
}

void IUINodeMap::onCampaignUnlock (Campaign* campaign)
{
	_campaignTextForNextPush = campaign->getText();
}
