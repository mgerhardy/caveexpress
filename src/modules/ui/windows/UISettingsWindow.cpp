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
	_texturesBig = assert_cast<UINodeButton*, UINode*>(getNode("textures_1", true));
	_texturesSmall = assert_cast<UINodeButton*, UINode*>(getNode("textures_2", true));

	_soundOn = assert_cast<UINodeButton*, UINode*>(getNode("soundmusic_1", true));
	_soundOff = assert_cast<UINodeButton*, UINode*>(getNode("soundmusic_2", true));
	_volume = assert_cast<UINodeSlider*, UINode*>(getNode("volume_1", true));
	_musicVolume = assert_cast<UINodeSlider*, UINode*>(getNode("musicvolume_1", true));

	_fullscreenOn = assert_cast<UINodeButton*, UINode*>(getNode("fullscreen_1", true));
	_fullscreenOff = assert_cast<UINodeButton*, UINode*>(getNode("fullscreen_2", true));

	_triggeraxisOn = assert_cast<UINodeButton*, UINode*>(getNode("triggeraxis_1", true));
	_triggeraxisOff = assert_cast<UINodeButton*, UINode*>(getNode("triggeraxis_2", true));

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
	panel->setPos(0.4, 0.1f);
	panel->setPadding(0.1f);
	panel->setSize(1.0f - panel->getX(), 1.0f - panel->getY());
	panel->setLayout(new UIVBoxLayout(10.0f / static_cast<float>(_frontend->getHeight()), false, NODE_ALIGN_CENTER));

	UINodeMainButton *button = new UINodeMainButton(_frontend, title);
	button->addListener(UINodeListenerPtr(new UISettingsSectionListener(this, panel)));
	_sectionPanel->add(button);

	return panel;
}

void UISettingsWindow::addSections()
{
	addToggleEntry(_settingsGfx, tr("Textures"), "textures",
			tr("Big"), new TextureModeListener("big", _frontend, _serviceProvider),
			tr("Small"), new TextureModeListener("small", _frontend, _serviceProvider));

	if (System.isFullscreenSupported()) {
		addToggleEntry(_settingsGfx, tr("Fullscreen"), "fullscreen",
				tr("On"), new FullscreenListener(_frontend, true),
				tr("Off"), new FullscreenListener(_frontend, false));
	}

	const int numDevices = SDL_GetNumAudioDevices(SDL_FALSE);
	if (numDevices == -1 || numDevices > 0) {
		addToggleEntry(_settingsSound, tr("Sound/Music"), "soundmusic",
				tr("On"), new SoundNodeListener(this, true),
				tr("Off"), new SoundNodeListener(this, false));
		addSliderEntry(_settingsSound, tr("Volume"), "volume", 0, 128, 1);
		addSliderEntry(_settingsSound, tr("Music volume"), "musicvolume", 0, 128, 1);
	}

	if (_frontend->hasMouse()) {
		addSliderEntry(_settingsInput, tr("Mouse speed"), "mousespeed", 0.1f, 3.0f, 0.1f);
	}

	addTextInputEntry(_settingsGame, tr("Username"), "name");

	addToggleEntry(_settingsInput, tr("Controller trigger"), "triggeraxis",
		tr("On"), new GameControllerTriggerNodeListener(true),
		tr("Off"), new GameControllerTriggerNodeListener(false));
	_noController = new UINodeLabel(_frontend, tr("No controller found"));
	_noController->setColor(colorGray);
	_noController->setFont(HUGE_FONT);
	_noController->setVisible(false);
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

void UISettingsWindow::addToggleEntry (UINode* parent, const std::string& title, const std::string& labelId, const std::string& option1, UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener)
{
	addLabelEntry(parent, title, labelId);

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

	parent->add(optionsPanel);
}

void UISettingsWindow::addSliderEntry (UINode* parent, const std::string& title, const std::string& configVar, float min, float max, float stepWidth)
{
	addLabelEntry(parent, title, configVar);

	UINodeSlider *sliderPanel = new UINodeSlider(_frontend, min, max, stepWidth);
	sliderPanel->setSize(0.1f, 0.06f);
	sliderPanel->addListener(UINodeListenerPtr(new ConfigVarListener(configVar, sliderPanel)));
	sliderPanel->setId(configVar + "_1");

	parent->add(sliderPanel);
}

void UISettingsWindow::addTextInputEntry (UINode* parent, const std::string& title, const std::string& configVar)
{
	addLabelEntry(parent, title, configVar);

	UINodeTextInput *textInput = new UINodeTextInput(_frontend, HUGE_FONT, 15);
	textInput->setBackgroundColor(colorWhite);
	textInput->setValue(Config.getConfigVar(configVar)->getValue());
	textInput->addListener(UINodeListenerPtr(new ConfigVarListener(configVar, textInput)));
	textInput->setId(configVar + "_1");

	parent->add(textInput);
}

void UISettingsWindow::addLabelEntry (UINode* parent, const std::string& title, const std::string& id)
{
	UINodeLabel *label = new UINodeLabel(_frontend, title);
	label->setId(id);
	label->setFont(HUGE_FONT);
	label->setColor(colorWhite);
	parent->add(label);
}
