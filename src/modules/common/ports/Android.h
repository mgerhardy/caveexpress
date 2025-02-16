#pragma once

#include "Unix.h"
#include "common/String.h"
#include "common/Application.h"
#include <string>
#include <vector>
#include <jni.h>
#include <android/log.h>

class Android: public Unix {
private:
	mutable JNIEnv* _env;
	jclass _cls;
	jobject _assetManager;

	jmethodID _showAds;
	jmethodID _hideAds;
	jmethodID _showFullscreenAds;
	jmethodID _openURL;
	jmethodID _hasItem;
	jmethodID _track;
	jmethodID _buyItem;
	jmethodID _isSmallScreen;
	jmethodID _minimize;
	jmethodID _getPaymentEntries;
	jmethodID _getLocale;
	jmethodID _achievementUnlocked;

	int _externalState;

	bool testException ();

	std::string getSystemProperty (const char *name) const;

public:
	Android ();
	virtual ~Android ();

	void init() override;

	bool supportsUserContent () const override { return false; }
	void notifyPaymentLoaded ();
	void tick (uint32_t deltaTime) override;
	int getAdHeight() const override;
	bool track (const std::string& hitType, const std::string& screenName) override;
	DirectoryEntries listDirectory (const std::string& basedir, const std::string& subdir = "") override;
	void showAds (bool show) override;
	bool showFullscreenAds () override;
	std::string getRateURL (const std::string& packageName) const override { return "market://details?id=" + packageName; }
	bool isFullscreenSupported () override { return false; }
	int openURL (const std::string& url, bool newWindow) const override;
	void exit (const std::string& reason, int errorCode) override;
	std::string getHomeDirectory () override;
	std::string getCurrentWorkingDir () override { return ""; }
	void achievementUnlocked (const std::string& id, bool increment) override;
	bool hasAchievement (const std::string& id) override;
	bool hasTouch () const override;
	bool quit () override;
	bool wantCursor () override;
	bool isSmallScreen (IFrontend*) override;
	bool supportFocusChange () override;
	std::string getLanguage () override;
	bool supportPayment () override;
	void getPaymentEntries (std::vector<PaymentEntry>& entries) override;
	bool hasItem (const std::string& id) override;
	bool buyItem (const std::string& id) override;
	bool hasMouseOrFinger () override;
	bool canDisableGameController () override { return true; }
	bool supportGooglePlay () { return true; }
};
