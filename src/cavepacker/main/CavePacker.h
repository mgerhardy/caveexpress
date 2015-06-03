#pragma once

#include "GameRegistry.h"
#include "client/ClientMap.h"
#include "cavepacker/server/map/Map.h"
#include "campaign/CampaignManager.h"
#include "campaign/persister/IGameStatePersister.h"

class CavePacker: public IGame, public IEntityVisitor {
private:
	IGameStatePersister* _persister;
	CampaignManager *_campaignManager;
	ClientMap *_clientMap;
	Map _map;
	IFrontend *_frontend;
	ServiceProvider* _serviceProvider;

	uint8_t getStars () const;
public:
	CavePacker();
	virtual ~CavePacker();

	void initUI (IFrontend* frontend, ServiceProvider& serviceProvider) override;
	void update (uint32_t deltaTime) override;
	std::string getMapName () override;
	int getMaxClients () override;
	void init (IFrontend *frontend, ServiceProvider& serviceProvider) override;
	void shutdown () override;
	int getPlayers () override;
	void connect (ClientId clientId) override;
	int disconnect (ClientId clientId) override;
	void mapReload () override;
	void mapShutdown () override;
	bool mapLoad (const std::string& map) override;
	IMapManager* getMapManager () override;

private:
	// IEntityVisitor
	bool visitEntity (IEntity *entity) override;
};

static GameRegisterStatic CAVEPACKER("cavepacker", GamePtr(new CavePacker()));
