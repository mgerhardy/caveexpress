#pragma once

#include "engine/client/ui/nodes/UINode.h"
#include "engine/client/ClientMap.h"
#include "engine/common/campaign/CampaignManager.h"
#include <string>
#include <vector>

//forward decl
class ServiceProvider;

// this node renders the map
class IUINodeMap: public UINode, public ICampaignManagerListener {
protected:
	ClientMap& _map;
	CampaignManager &_campaignManager;
	std::string _campaignTextForNextPush;
	std::vector<std::string> _players;
	std::string _title;

	void onCampaignUnlock (Campaign* campaign) override;
public:
	IUINodeMap (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, int x, int y, int width, int height, ClientMap& map);
	virtual ~IUINodeMap ();

	void setPlayerList (const std::vector<std::string>& players);
	void setTitle (const std::string& title);
	void start();

	// UINode
	void update (uint32_t deltaTime) override;
	void render (int x, int y) const override;
	bool onPush () override;
	bool onPop () override;

	bool initWaitingForPlayer ();

	void setMapRect (int x, int y, int w, int h);
	ClientMap& getMap();
};

inline ClientMap& IUINodeMap::getMap()
{
	return _map;
}

inline void IUINodeMap::setPlayerList (const std::vector<std::string>& players)
{
	_players = players;
}

inline void IUINodeMap::setTitle (const std::string& title)
{
	_title = title;
}

inline void IUINodeMap::start()
{
	_map.start();
	displayText(_title);
}

inline bool IUINodeMap::initWaitingForPlayer ()
{
	return _map.initWaitingForPlayer();
}
