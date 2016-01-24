#include "UISettingsWindow.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeBackToRootButton.h"
#include "ui/nodes/UINodeCheckbox.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSlider.h"
#include "ui/nodes/UINodeButtonImage.h"
#include "ui/nodes/UINodeButtonText.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/layouts/UIVBoxLayout.h"
#include "ui/windows/UIWindow.h"
#include "ui/windows/listener/ConfigVarListener.h"
#include "ui/nodes/UINodeSettingsBackground.h"
#include "ui/windows/modeselection/ModeSetListener.h"
#include "sound/Sound.h"
#include "common/IFrontend.h"
#include "service/ServiceProvider.h"
#include "common/System.h"
#include "network/INetwork.h"

#include "ui/windows/listener/TextureModeListener.h"
#include "ui/windows/listener/GameControllerTriggerNodeListener.h"
#include "ui/windows/listener/SoundNodeListener.h"
#include "ui/windows/listener/FullscreenListener.h"

#include <SDL.h>

class UISettingsSectionListener : public UINodeListener {
private:
	UISettingsWindow* _window;
	UINode* _activeSection;

	inline void vis(UINode* node) const {
		node->setVisible(_activeSection == node);
	}
public:
	UISettingsSectionListener(UISettingsWindow* window, UINode* section) :
			_window(window), _activeSection(section) {
	}

	void onClick () override {
		vis(_window->_settingsGame);
		vis(_window->_settingsGfx);
		vis(_window->_settingsInput);
		vis(_window->_settingsSound);
	}
};

UISettingsWindow::UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIWindow(UI_WINDOW_SETTINGS, frontend, WINDOW_FLAG_MODAL), _sectionPanel(nullptr), _settingsGfx(nullptr), _settingsSound(nullptr), _settingsGame(
				nullptr), _settingsInput(nullptr), _serviceProvider(serviceProvider), _controllerNode(nullptr), _noController(nullptr), _texturesBig(
				nullptr), _texturesSmall(nullptr), _soundOn(nullptr), _soundOff(nullptr), _fullscreenOn(nullptr), _fullscreenOff(nullptr), _triggeraxisOn(
				nullptr), _triggeraxisOff(nullptr), _volume(nullptr), _musicVolume(nullptr) {
}

void UISettingsWindow::init()
{
	UINodeSettingsBackground* background = new UINodeSettingsBackground(_frontend);
	add(background);

	_sectionPanel = new UINode(_frontend, "settings-panel-sections");
	_sectionPanel->setMargin(0.0f, 0.1f);
	_sectionPanel->setSize(0.4f);
	_sectionPanel->setAlignment(NODE_ALIGN_MIDDLE);
	_sectionPanel->setLayout(new UIVBoxLayout(20.0f / static_cast<float>(_frontend->getHeight()), true));

	_settingsGfx = configureSection("settings-gfx", tr("Graphics"), true);
	_settingsSound = configureSection("settings-sound", tr("Sound"), false);
	_settingsGame = configureSection("settings-game", tr("Game"), false);
	_settingsInput = configureSection("settings-input", tr("Input"), false);

	// left
	add(_sectionPanel);

	addSections();

	// right
	add(_settingsGfx);
	add(_settingsSound);
	add(_settingsGame);
	add(_settingsInput);

	_controllerNode = getNode("triggeraxis_panel", true);
	_texturesBig = (UINodeButton*)getNode("textures_1", true);
	_texturesSmall = (UINodeButton*)getNode("textures_2", true);

	_soundOn = (UINodeButton*)getNode("soundmusic_1", true);
	_soundOff = (UINodeButton*)getNode("soundmusic_2", true);
	_volume = (UINodeSlider*)getNode("volume_1", true);
	_musicVolume = (UINodeSlider*)getNode("musicvolume_1", true);

	_fullscreenOn = (UINodeButton*)getNode("fullscreen_1", true);
	_fullscreenOff = (UINodeButton*)getNode("fullscreen_2", true);

	_triggeraxisOn = (UINodeButton*)getNode("triggeraxis_1", true);
	_triggeraxisOff = (UINodeButton*)getNode("triggeraxis_2", true);

	if (!wantBackButton())
		return;

	UINodeBackButton *backButton = new UINodeBackButton(_frontend);
	const float gapBack = std::max(0.01f, getScreenPadding());
	backButton->alignTo(background, NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT, gapBack);
	add(backButton);
}

UINode* UISettingsWindow::configureSection(const std::string& id, const std::string& title, bool visible)
{
	UINode* panel = new UINode(_frontend, "settings-sound");
	panel->setVisible(visible);
	panel->setPos(0.4, 0.0f);
	panel->setSize(1.0f - panel->getWidth(), 1.0f - panel->getHeight());
	panel->setLayout(new UIVBoxLayout(10.0f / static_cast<float>(_frontend->getHeight()), false, NODE_ALIGN_CENTER));

	UINodeMainButton *button = new UINodeMainButton(_frontend, title);
	button->addListener(UINodeListenerPtr(new UISettingsSectionListener(this, panel)));
	_sectionPanel->add(button);

	return panel;
}

void UISettingsWindow::addSections()
{
	addSection(_settingsGfx, tr("Textures"), "textures",
			tr("Big"), new TextureModeListener("big", _frontend, _serviceProvider),
			tr("Small"), new TextureModeListener("small", _frontend, _serviceProvider));

	if (System.isFullscreenSupported()) {
		addSection(_settingsGfx, tr("Fullscreen"), "fullscreen",
				tr("On"), new FullscreenListener(_frontend, true),
				tr("Off"), new FullscreenListener(_frontend, false));
	}

	const int numDevices = SDL_GetNumAudioDevices(SDL_FALSE);
	if (numDevices == -1 || numDevices > 0) {
		addSection(_settingsSound, tr("Sound/Music"), "soundmusic",
				tr("On"), new SoundNodeListener(this, true),
				tr("Off"), new SoundNodeListener(this, false));
		addSection(_settingsSound, tr("Volume"), "volume", "volume", 0, 128, 1);
		addSection(_settingsSound, tr("Music volume"), "musicvolume", "musicvolume", 0, 128, 1);
	}

	if (_frontend->hasMouse()) {
		addSection(_settingsInput, tr("Mouse speed"), "mousespeed", "mousespeed", 0.1f, 3.0f, 0.1f);
	}

	addSection(_settingsInput, tr("Controller trigger axis"), "triggeraxis",
		tr("On"), new GameControllerTriggerNodeListener(true),
		tr("Off"), new GameControllerTriggerNodeListener(false));
	_noController = new UINodeLabel(_frontend, tr("No game controller attached"));
	_noController->setColor(colorGray);
	_noController->setFont(HUGE_FONT);
	_noController->setVisible(false);
//	_noController->centerUnder(getNode("triggeraxis"), 0.03f);
	_settingsInput->add(_noController);
}

void UISettingsWindow::update (uint32_t time)
{
	UIWindow::update(time);

	const bool visible = UI::get().hasController();
	_controllerNode->setVisible(visible);
	_noController->setVisible(!visible);

	const bool sound = Config.isSoundEnabled();
	_volume->setEnabled(sound);
	_musicVolume->setEnabled(sound);

	UIWINDOW_SETTINGS_COLOR(_serviceProvider.getTextureDefinition().getTextureSize() == "big", _texturesBig, _texturesSmall)
	UIWINDOW_SETTINGS_COLOR(sound, _soundOn, _soundOff)
	UIWINDOW_SETTINGS_COLOR(Config.isFullscreen(), _fullscreenOn, _fullscreenOff)
	UIWINDOW_SETTINGS_COLOR(Config.isGameControllerTriggerActive(), _triggeraxisOn, _triggeraxisOff)
}

void UISettingsWindow::addSection (UINode* parent, const std::string& title, const std::string& labelId, const std::string& option1, UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener)
{
	UINodeLabel *label = new UINodeLabel(_frontend, title);
	label->setId(labelId);
	label->setFont(HUGE_FONT);
	label->setColor(colorWhite);

	UINode *optionsPanel = new UINode(_frontend, labelId + "_panel");
	optionsPanel->setLayout(new UIHBoxLayout(0.02f));

	const BitmapFontPtr& fontPtr = getFont(HUGE_FONT);
	UINodeButtonText *option1Text = new UINodeButtonText(_frontend, option1);
	option1Text->addListener(UINodeListenerPtr(option1Listener));
	option1Text->setId(labelId + "_1");
	option1Text->setFont(fontPtr);
	optionsPanel->add(option1Text);

	UINodeButtonText *option2Text = new UINodeButtonText(_frontend, option2);
	option2Text->addListener(UINodeListenerPtr(option2Listener));
	option2Text->setId(labelId + "_2");
	option2Text->setFont(fontPtr);
	optionsPanel->add(option2Text);

	parent->add(label);
	parent->add(optionsPanel);
}

void UISettingsWindow::addSection (UINode* parent, const std::string& title, const std::string& labelId, const std::string& configVar, float min, float max, float stepWidth)
{
	UINodeLabel *label = new UINodeLabel(_frontend, title);
	label->setId(labelId);
	label->setFont(HUGE_FONT);
	label->setColor(colorWhite);

	UINodeSlider *sliderPanel = new UINodeSlider(_frontend, min, max, stepWidth);
	sliderPanel->setSize(0.1f, 0.06f);
	sliderPanel->addListener(UINodeListenerPtr(new ConfigVarListener(configVar, sliderPanel)));
	sliderPanel->setId(labelId + "_1");

	parent->add(label);
	parent->add(sliderPanel);
}
