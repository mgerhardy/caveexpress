#pragma once

#include "engine/GameRegistry.h"
#include "engine/client/ClientMap.h"
#include "caveanddiamonds/server/map/Map.h"
#include "engine/common/campaign/CampaignManager.h"
#include "engine/common/campaign/persister/IGameStatePersister.h"

class CaveAndDiamonds: public IGame, public IEntityVisitor {
private:
	IGameStatePersister* _persister;
	CampaignManager *_campaignManager;
	ClientMap *_clientMap;
	Map _map;
	IFrontend *_frontend;
	ServiceProvider* _serviceProvider;

	uint8_t getStars () const;
public:
	CaveAndDiamonds();
	virtual ~CaveAndDiamonds();

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

private:
	// IEntityVisitor
	bool visitEntity (IEntity *entity) override;
};

static GameRegisterStatic CAVEEXPRESS("cavepacker", GamePtr(new CaveAndDiamonds()));
