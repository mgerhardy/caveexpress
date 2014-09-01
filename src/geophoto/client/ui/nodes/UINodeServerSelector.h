#pragma once

#include "UINodeSelector.h"
#include "shared/network/INetwork.h"

struct ServerEntry {
	ServerEntry (const std::string& _name, const std::string& _host, int _port, const std::string& _mapName,
			int _playerCount, int _maxPlayerCount) :
			name(_name), host(_host), port(_port), mapName(_mapName), playerCount(_playerCount), maxPlayerCount(
					_maxPlayerCount)
	{
	}
	std::string name;
	std::string host;
	std::string mapName;
	int port;
	int playerCount;
	int maxPlayerCount;
};

// this node search for servers and presents then in a list to connect to them
class UINodeServerSelector: public UINodeBackgroundSelector<ServerEntry> {
public:
	UINodeServerSelector (IFrontend *frontend, int cols = 5, int rows = 2);
	virtual ~UINodeServerSelector ();

	void addServer (const std::string& host, const std::string& name, const std::string& mapName, int port,
			int playerCount, int maxPlayerCount);

	// UINodeSelector
	bool onSelect (const ServerEntry& data) override;
	TexturePtr getIcon (const ServerEntry& data) const override;
	std::string getText (const ServerEntry& data) const override;

	// UINode
	bool onPush () override;
};
