#pragma once

#include "ui/windows/UIWindow.h"

// forward decl
class ServiceProvider;
class UINodeLabel;
class UINodeButton;
class UINodeSlider;

#define UIWINDOW_SETTINGS_COLOR(state, one, two) \
	if (state) { \
		one->setColor(colorBlack); \
		two->setColor(colorGray); \
	} else { \
		two->setColor(colorBlack); \
		one->setColor(colorGray); \
	}

class UISettingsWindow: public UIWindow {
protected:
	UINode* _background;
	ServiceProvider& _serviceProvider;
	UINode* _controllerNode;
	UINodeLabel* _noController;

	UINodeButton* _texturesBig;
	UINodeButton* _texturesSmall;

	UINodeButton* _soundOn;
	UINodeButton* _soundOff;

	UINodeButton* _fullscreenOn;
	UINodeButton* _fullscreenOff;

	UINodeButton* _triggeraxisOn;
	UINodeButton* _triggeraxisOff;

	UINodeSlider* _volume;
	UINodeSlider* _musicVolume;

	UINode* addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& labelId, const std::string& option1,
			UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener);
	UINode* addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& labelId, const std::string& configVar, float min, float max, float stepWidth);
	virtual UINode* addSections();
public:
	UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	void init();

	virtual void update(uint32_t time) override;
};
