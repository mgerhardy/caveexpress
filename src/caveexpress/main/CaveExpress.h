#pragma once

#include "GameRegistry.h"
#include "campaign/CampaignManager.h"
#include "campaign/persister/SQLitePersister.h"
#include "caveexpress/server/map/Map.h"

class ClientMap;

namespace caveexpress {

class CaveExpress: public IGame, public IEntityVisitor {
private:
	IGameStatePersister* _persister;
	CampaignManager *_campaignManager;
	ClientMap *_clientMap;
	Map _map;
	int32_t _updateEntitiesTime;
	IFrontend *_frontend;
	ServiceProvider *_serviceProvider;
	char _connectedClients;
	int _packageCount;
	int32_t _loadDelay;
	std::string _loadDelayName;
public:
	CaveExpress();
	virtual ~CaveExpress();

	void initUI (IFrontend* frontend, ServiceProvider& serviceProvider) override;
	void update (uint32_t deltaTime) override;
	std::string getMapName () override;
	void init (IFrontend *frontend, ServiceProvider& serviceProvider) override;
	void shutdown () override;
	int getPlayers () override;
	void connect (ClientId clientId) override;
	int disconnect (ClientId clientId) override;
	void mapReload () override;
	void mapShutdown () override;
	bool mapLoad (const std::string& map) override;
	IMapManager* getMapManager () override;
	Map& getMap ();

private:
	// IEntityVisitor
	bool visitEntity (IEntity *entity) override;
};

}
