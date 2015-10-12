#pragma once

#include "ui/UI.h"
#include "common/Application.h"
#include "common/ConfigManager.h"

class RatePopupCallback: public UIPopupCallback {
private:
	IFrontend* _frontend;
public:
	explicit RatePopupCallback(IFrontend* frontend) :
		_frontend(frontend) {
	}

	void onOk() override {
		const std::string& packageName = Singleton<Application>::getInstance().getPackageName();
		const std::string url = System.getRateURL(packageName);
		if (!url.empty()) {
			_frontend->minimize();
			System.openURL(url, true);
			Config.getConfigVar("alreadyrated")->setValue("true");
		}

		UIPopupCallback::onOk();
	}

	void onLater() override {
		// otherwise we would push the popup right after pop
		Config.increaseCounter("launchcount");
		UIPopupCallback::onLater();
	}

	void onCancel() override {
		Config.getConfigVar("alreadyrated")->setValue("true");
		UIPopupCallback::onCancel();
	}
};
