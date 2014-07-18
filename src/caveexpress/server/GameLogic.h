#pragma once

#include "caveexpress/server/map/Map.h"
#include "engine/common/network/INetwork.h"
#include <vector>

// forward decl
class CaveMapTile;
class NPCPackage;
class NPCAttacking;
class NPC;
class Package;
class IFrontend;
class ServiceProvider;
class ICampaignManager;

class GameLogic: public IEntityVisitor {
private:
	int32_t _updateEntitiesTime;

	Map _map;

	IFrontend *_frontend;
	ServiceProvider *_serviceProvider;
	ICampaignManager *_campaignManager;

	char _connectedClients;

	int _packageCount;

	int32_t _loadDelay;
	std::string _loadDelayName;

public:
	GameLogic ();
	virtual ~GameLogic ();

	void init (IFrontend *frontend, ServiceProvider *serviceProvider, ICampaignManager *campaignManager);
	void shutdown ();
	/**
	 * @param[in] deltaTime The milliseconds since the last frame was executed
	 * @return If @c true is returned, the next map in the campaign is going to be started.
	 * @c false is returned if the map is still running or whenever it failed.
	 */
	bool update (uint32_t deltaTime);

	bool loadMap (const std::string& mapName);
	// return the amount of left clients after the removal
	int disconnect (ClientId clientId);
	void connect (ClientId clientId);
	Map& getMap ();

private:
	void loadDelayed (const std::string& name);

	// IEntityVisitor
	bool visitEntity (IEntity *entity) override;
};
