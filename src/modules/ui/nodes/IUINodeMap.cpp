#include "IUINodeMap.h"
#include "common/CommandSystem.h"
#include "common/Commands.h"
#include "common/EventHandler.h"
#include "common/Direction.h"
#include "client/commands/CmdConnect.h"
#include "client/commands/CmdDisconnect.h"
#include "ui/UI.h"
#include "ui/BitmapFont.h"
#include "ui/windows/IUIMapWindow.h"
#include "client/network/ChangeAnimationHandler.h"
#include "client/network/MapRestartHandler.h"
#include "client/network/PauseHandler.h"
#include "client/network/UpdateEntityHandler.h"
#include "client/network/RemoveEntityHandler.h"
#include "client/network/SoundHandler.h"
#include "client/network/BackToMainHandler.h"
#include "client/network/RumbleHandler.h"
#include "client/network/PlayerListHandler.h"
#include "client/network/TextMessageHandler.h"
#include "network/ProtocolHandlerRegistry.h"
#include "service/ServiceProvider.h"
#include "common/IFrontend.h"
#include "client/commands/CmdMove.h"
#include "client/commands/CmdZoom.h"
#include "campaign/CampaignManager.h"
#include "client/network/CloseMapHandler.h"
#include "client/network/HudLoadMapHandler.h"
#include "client/network/HudMapSettingsHandler.h"
#include "client/network/InitWaitingMapHandler.h"
#include "client/network/StartMapHandler.h"
#include "client/network/UpdateHitpointsHandler.h"
#include "client/network/UpdateLivesHandler.h"
#include "client/network/UpdatePointsHandler.h"
#include "client/network/TimeRemainingHandler.h"
#include "client/network/FinishedMapHandler.h"

IUINodeMap::IUINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, ClientMap& map) :
		UINode(frontend), _map(map), _campaignManager(campaignManager)
{
	Commands.registerCommand(CMD_CL_CONNECT, new CmdConnect(&_map, serviceProvider));
	Commands.registerCommand(CMD_CL_DISCONNECT, new CmdDisconnect(serviceProvider));
	Commands.registerCommand(CMD_MOVE_UP, new CmdMove(_map, DIRECTION_UP));
	Commands.registerCommand(CMD_MOVE_DOWN, new CmdMove(_map, DIRECTION_DOWN));
	Commands.registerCommand(CMD_ZOOM, new CmdZoom(_map));
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
	r.registerClientHandler(protocol::PROTO_STARTMAP, new StartClientMapHandler());
	r.registerClientHandler(protocol::PROTO_UPDATEHITPOINTS, new UpdateHitpointsHandler());
	r.registerClientHandler(protocol::PROTO_UPDATELIVES, new UpdateLivesHandler(campaignManager));
	r.registerClientHandler(protocol::PROTO_UPDATEPOINTS, new UpdatePointsHandler());
	r.registerClientHandler(protocol::PROTO_TIMEREMAINING, new TimeRemainingHandler());
	r.registerClientHandler(protocol::PROTO_FINISHEDMAP, new FinishedMapHandler(_map));

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
	Commands.removeCommand(CMD_ZOOM);

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
	r.unregisterClientHandler(protocol::PROTO_PAUSE);
	r.unregisterClientHandler(protocol::PROTO_UPDATEHITPOINTS);
	r.unregisterClientHandler(protocol::PROTO_UPDATELIVES);
	r.unregisterClientHandler(protocol::PROTO_UPDATEPOINTS);
	r.unregisterClientHandler(protocol::PROTO_TIMEREMAINING);
	r.unregisterClientHandler(protocol::PROTO_FINISHEDMAP);
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
		_map.render();
	UINode::render(0, 0);

	if (_map.isStarted())
		return;
	const BitmapFontPtr& font = getFont(HUGE_FONT);
	y += getRenderHeight() / 10;
	x += getRenderWidth() / 10;

	y += font->print(tr("Players:"), colorWhite, x, y) + font->getCharHeight();
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

void IUINodeMap::onCampaignUnlock (Campaign* oldCampaign, Campaign* newCampaign)
{
	_campaignTextForNextPush = newCampaign->getText();
}

bool IUINodeMap::isActive() const
{
	if (UINode::isActive())
		return true;
	return _map.isActive();
}
