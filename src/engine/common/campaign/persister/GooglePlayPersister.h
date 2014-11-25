#pragma once

#include "IGameStatePersister.h"
#include <SDL_platform.h>

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
	jmethodID _loadCampaign;
	jmethodID _saveCampaign;
	jmethodID _persisterInit;
	jmethodID _persisterConnect;
	jmethodID _persisterDisconnect;
#endif

	void connect();
	void disconnect();

	bool testException ();
public:
	GooglePlayPersister(IGameStatePersister* delegate);
	virtual ~GooglePlayPersister();

	bool init() override;
	bool saveCampaign (Campaign*) override;
	bool loadCampaign (Campaign*) override;
	bool reset () override;
	bool resetCampaign (Campaign*) override;
};
