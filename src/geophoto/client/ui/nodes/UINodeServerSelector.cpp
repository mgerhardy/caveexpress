#include "UINodeServerSelector.h"
#include "shared/CommandSystem.h"
#include "shared/constants/Commands.h"
#include "shared/Logger.h"

UINodeServerSelector::UINodeServerSelector (IFrontend *frontend, int cols, int rows) :
		UINodeBackgroundSelector<ServerEntry>(frontend, cols, rows)
{
	reset();
}

UINodeServerSelector::~UINodeServerSelector ()
{
}

void UINodeServerSelector::addServer (const std::string &host, const std::string& name, const std::string& mapName,
		int port, int playerCount, int maxPlayerCount)
{
	addData(ServerEntry(name, host, port, mapName, playerCount, maxPlayerCount));
}

bool UINodeServerSelector::onSelect (const ServerEntry& data)
{
	const std::string connect = CMD_CL_CONNECT " " + data.host + " " + string::toString(data.port);
	info(LOG_CLIENT, "connect via '" + connect + "'");
	Commands.executeCommandLine(connect);
	return true;
}

TexturePtr UINodeServerSelector::getIcon (const ServerEntry& data) const
{
	return loadTexture("icon-server");
}

std::string UINodeServerSelector::getText (const ServerEntry& data) const
{
	return "server: " + data.name;
}

bool UINodeServerSelector::onPush ()
{
	reset();
	Commands.executeCommandLine(CMD_CL_PINGSERVERS);
	return true;
}
