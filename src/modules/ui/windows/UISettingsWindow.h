#pragma once

#include "ui/windows/UIWindow.h"

// forward decl
class ServiceProvider;
class UINodeLabel;
class UINodeButton;
class UINodeSlider;

#define UIWINDOW_SETTINGS_COLOR(state, one, two) \
	if (state) { \
		if (one) \
			one->setColor(colorBlack); \
		if (two) \
			two->setColor(colorGray); \
	} else { \
		if (two) \
			two->setColor(colorBlack); \
		if (one) \
			one->setColor(colorGray); \
	}

class UISettingsWindow: public UIWindow {
protected:
	friend class UISettingsSectionListener;
	UINode* _sectionPanel;
	UINode* _settingsGfx;
	UINode* _settingsSound;
	UINode* _settingsGame;
	UINode* _settingsInput;
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

	// add sections here - games can override this to populate the game section or even add new ones
	virtual void addSections();

	// set up a new section including the button to activate it
	// the returned UINode can then be populated by the add*Entry methods.
	UINode* configureSection (const std::string& id, const std::string& title, bool visible = false);

	// headline label entry for the config option
	void addLabelEntry (UINode* parent, const std::string& title, const std::string& id);
	// adds text input nodes
	void addTextInputEntry (UINode* parent, const std::string& title, const std::string& configVar);
	// adds buttons for on/off
	void addToggleEntry (UINode* parent, const std::string& title, const std::string& labelId, const std::string& option1,
			UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener);
	// adds slider
	void addSliderEntry (UINode* parent, const std::string& title, const std::string& configVar, float min, float max, float stepWidth);
public:
	UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider);
	// set up the window nodes
	void init();

	virtual void update(uint32_t time) override;
};
