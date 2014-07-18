#pragma once

#include "engine/GameRegistry.h"
#include "caveexpress/server/GameLogic.h"
#include "engine/common/campaign/CampaignManager.h"
#include "engine/common/campaign/persister/SQLitePersister.h"

class ClientMap;

class CaveExpress: public IGame {
private:
	GameLogic _game;
	IGameStatePersister* _persister;
	CampaignManager *_campaignManager;
	ClientMap *_map;

public:
	CaveExpress();
	virtual ~CaveExpress();

	void initUI (IFrontend* frontend, ServiceProvider& serviceProvider) override;
	void initSoundCache () override;
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
	UIWindow* createPopupWindow (IFrontend* frontend, const std::string& text, int flags, UIPopupCallbackPtr callback) override;
};

static GameRegisterStatic CAVEEXPRESS("caveexpress", GamePtr(new CaveExpress()));
