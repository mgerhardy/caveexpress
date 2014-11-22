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
#ifdef GOOGLEPLAY_ACTIVE
	mutable JNIEnv* _env;
	jclass _cls;
	jmethodID _loadCampaign;
	jmethodID _saveCampaign;
#endif

	bool testException ();
public:
	GooglePlayPersister();
	virtual ~GooglePlayPersister();

	bool init() override;
	bool saveCampaign (Campaign*) override;
	bool loadCampaign (Campaign*) override;
	bool reset () override;
	bool resetCampaign (Campaign*) override;
};
