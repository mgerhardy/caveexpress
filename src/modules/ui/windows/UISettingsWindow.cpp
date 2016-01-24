#include "UISettingsWindow.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeBackToRootButton.h"
#include "ui/nodes/UINodeCheckbox.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSlider.h"
#include "ui/nodes/UINodeButtonImage.h"
#include "ui/nodes/UINodeButtonText.h"
#include "ui/layouts/UIHBoxLayout.h"
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

UISettingsWindow::UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIWindow(UI_WINDOW_SETTINGS, frontend, WINDOW_FLAG_MODAL), _background(nullptr), _serviceProvider(serviceProvider), _controllerNode(
				nullptr), _noController(nullptr), _texturesBig(nullptr), _texturesSmall(nullptr), _soundOn(nullptr), _soundOff(nullptr), _fullscreenOn(
				nullptr), _fullscreenOff(nullptr), _triggeraxisOn(nullptr), _triggeraxisOff(nullptr) {
}

void UISettingsWindow::init()
{
	_background = new UINodeSettingsBackground(_frontend);
	add(_background);

	addSections();

	if (!wantBackButton())
		return;

	UINodeBackButton *backButton = new UINodeBackButton(_frontend);
	const float gapBack = std::max(0.01f, getScreenPadding());
	backButton->alignTo(_background, NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT, gapBack);
	add(backButton);
}

UINode* UISettingsWindow::addSections()
{
	UINode* last = nullptr;
	last = addSection(last, _background, tr("Textures"), "textures",
			tr("Big"), new TextureModeListener("big", _frontend, _serviceProvider),
			tr("Small"), new TextureModeListener("small", _frontend, _serviceProvider));

	const int numDevices = SDL_GetNumAudioDevices(SDL_FALSE);
	if (numDevices == -1 || numDevices > 0) {
		last = addSection(last, nullptr, tr("Sound/Music"), "soundmusic",
				tr("On"), new SoundNodeListener(this, true),
				tr("Off"), new SoundNodeListener(this, false));
	}

	if (System.isFullscreenSupported()) {
		last = addSection(last, nullptr, tr("Fullscreen"), "fullscreen",
				tr("On"), new FullscreenListener(_frontend, true),
				tr("Off"), new FullscreenListener(_frontend, false));
	}

	if (_frontend->hasMouse()) {
		last = addSection(last, nullptr, tr("Mouse speed"), "mousespeed", "mousespeed", 0.1f, 3.0f, 0.1f);
	}

	UINode* center = last;
	last = addSection(center, nullptr, tr("Controller trigger axis"), "triggeraxis",
		tr("On"), new GameControllerTriggerNodeListener(true),
		tr("Off"), new GameControllerTriggerNodeListener(false));
	_controllerNode = last;
	_noController = new UINodeLabel(_frontend, tr("No game controller attached"));
	_noController->setColor(colorGray);
	_noController->setFont(HUGE_FONT);
	_noController->setVisible(false);
	_noController->centerUnder(getNode("triggeraxis"), 0.03f);
	add(_noController);

	_texturesBig = (UINodeButton*)getNode("textures_1");
	_texturesSmall = (UINodeButton*)getNode("textures_2");

	_soundOn = (UINodeButton*)getNode("soundmusic_1");
	_soundOff = (UINodeButton*)getNode("soundmusic_2");

	_fullscreenOn = (UINodeButton*)getNode("fullscreen_1");
	_fullscreenOff = (UINodeButton*)getNode("fullscreen_2");

	_triggeraxisOn = (UINodeButton*)getNode("triggeraxis_1");
	_triggeraxisOff = (UINodeButton*)getNode("triggeraxis_2");

	return last;
}

void UISettingsWindow::update (uint32_t time)
{
	UIWindow::update(time);
	const bool visible = UI::get().hasController();
	_controllerNode->setVisible(visible);
	_noController->setVisible(!visible);

	UIWINDOW_SETTINGS_COLOR(_serviceProvider.getTextureDefinition().getTextureSize() == "big", _texturesBig, _texturesSmall)
	UIWINDOW_SETTINGS_COLOR(Config.isSoundEnabled(), _soundOn, _soundOff)
	UIWINDOW_SETTINGS_COLOR(Config.isFullscreen(), _fullscreenOn, _fullscreenOff)
	UIWINDOW_SETTINGS_COLOR(Config.isGameControllerTriggerActive(), _triggeraxisOn, _triggeraxisOff)
}

UINode* UISettingsWindow::addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& labelId, const std::string& option1, UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener)
{
	UINodeLabel *label = new UINodeLabel(_frontend, title);
	label->setId(labelId);
	label->setFont(HUGE_FONT);
	label->setColor(colorWhite);
	if (background != nullptr) {
		label->alignTo(background, NODE_ALIGN_CENTER | NODE_ALIGN_TOP, 0.15f);
	} else {
		SDL_assert(centerUnderNode);
		label->centerUnder(centerUnderNode, 0.03f);
	}
	add(label);
	UINode *optionsPanel = new UINode(_frontend, title);
	{
		UIHBoxLayout *hlayout = new UIHBoxLayout(0.02f);
		optionsPanel->setLayout(hlayout);
		{
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
		}
	}
	add(optionsPanel);
	optionsPanel->centerUnder(label);
	return optionsPanel;
}

UINode* UISettingsWindow::addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& labelId, const std::string& configVar, float min, float max, float stepWidth)
{
	UINodeLabel *label = new UINodeLabel(_frontend, title);
	label->setId(labelId);
	label->setFont(HUGE_FONT);
	label->setColor(colorWhite);
	if (background != nullptr) {
		label->alignTo(background, NODE_ALIGN_CENTER | NODE_ALIGN_TOP, 0.2f);
	} else {
		SDL_assert(centerUnderNode);
		label->centerUnder(centerUnderNode, 0.03f);
	}
	add(label);
	UINodeSlider *sliderPanel = new UINodeSlider(_frontend, min, max, stepWidth);
	const float height = 0.06f;
	const float sliderWidth = 0.1f;
	sliderPanel->setSize(sliderWidth, height);
	ConfigVarListener* listener = new ConfigVarListener(configVar, sliderPanel);
	sliderPanel->addListener(UINodeListenerPtr(listener));
	sliderPanel->setId(labelId + "_1");
	add(sliderPanel);
	sliderPanel->centerUnder(label);
	return sliderPanel;
}
