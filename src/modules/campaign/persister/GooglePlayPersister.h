#pragma once

#include "IGameStatePersister.h"
#include <SDL_platform.h>
#include <string>

#ifdef __ANDROID__
#define GOOGLEPLAY_ACTIVE
#endif

#ifdef GOOGLEPLAY_ACTIVE
#include <jni.h>
#endif

class GooglePlayPersister: public IGameStatePersister {
private:
	IGameStatePersister* _delegate;

#ifdef GOOGLEPLAY_ACTIVE
	mutable JNIEnv* _env;
	jclass _cls;
	jmethodID _loadGameState;
	jmethodID _saveGameState;
	jmethodID _persisterInit;
	jmethodID _persisterConnect;
	jmethodID _persisterDisconnect;
	jmethodID _showLeaderBoard;
	jmethodID _showAchievements;
	jmethodID _addPointsToLeaderBoard;
#endif

	void connect();
	void disconnect();
	void showAchievements();
	void showLeaderBoard(const std::string& boardId);

	void upload();
	void download();

	bool testException ();
public:
	explicit GooglePlayPersister(IGameStatePersister* delegate);
	virtual ~GooglePlayPersister();

	bool init() override;
	bool saveCampaign (Campaign*) override;
	bool loadCampaign (Campaign*) override;
	bool reset () override;
	bool resetCampaign (Campaign*) override;
};
