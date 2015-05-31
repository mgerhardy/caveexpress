#include "UISettingsWindow.h"
#include "client/ui/nodes/UINodeBackButton.h"
#include "client/ui/nodes/UINodeBackToRootButton.h"
#include "client/ui/nodes/UINodeCheckbox.h"
#include "client/ui/nodes/UINodeLabel.h"
#include "client/ui/nodes/UINodeButtonImage.h"
#include "client/ui/nodes/UINodeButtonText.h"
#include "client/ui/layouts/UIHBoxLayout.h"
#include "client/ui/windows/UIWindow.h"
#include "client/ui/nodes/UINodeSettingsBackground.h"
#include "client/ui/windows/modeselection/ModeSetListener.h"
#include "client/sound/Sound.h"
#include "common/IFrontend.h"
#include "common/ServiceProvider.h"
#include "common/System.h"
#include "common/network/INetwork.h"

#include "client/ui/windows/listener/TextureModeListener.h"
#include "client/ui/windows/listener/JoystickNodeListener.h"
#include "client/ui/windows/listener/SoundNodeListener.h"
#include "client/ui/windows/listener/FullscreenListener.h"

#include <SDL_platform.h>

UISettingsWindow::UISettingsWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIWindow(UI_WINDOW_SETTINGS, frontend, WINDOW_FLAG_MODAL), _background(nullptr), _serviceProvider(serviceProvider)
{
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
	last = addSection(last, _background, tr("Textures"),
			tr("Big"), new TextureModeListener("big", _frontend, _serviceProvider),
			tr("Small"), new TextureModeListener("small", _frontend, _serviceProvider));

	last = addSection(last, nullptr, tr("Sound/Music"),
			tr("On"), new SoundNodeListener(this, true),
			tr("Off"), new SoundNodeListener(this, false));

	if (System.isFullscreenSupported()) {
		last = addSection(last, nullptr, tr("Fullscreen"),
				tr("On"), new FullscreenListener(_frontend, true),
				tr("Off"), new FullscreenListener(_frontend, false));
	}

	if (System.canDisableJoystick() && _frontend->hasJoystick()) {
		last = addSection(last, nullptr, tr("Joystick"),
			tr("On"), new JoystickNodeListener(true),
			tr("Off"), new JoystickNodeListener(false));
	}

	return last;
}

UINode* UISettingsWindow::addSection (UINode* centerUnderNode, UINode* background, const std::string& title, const std::string& option1, UINodeListener* option1Listener, const std::string& option2, UINodeListener* option2Listener)
{
	UINodeLabel *label = new UINodeLabel(_frontend, title);
	label->setFont(HUGE_FONT);
	label->setColor(colorWhite);
	if (background != nullptr) {
		label->alignTo(background, NODE_ALIGN_CENTER | NODE_ALIGN_TOP, 0.2f);
	} else {
		assert(centerUnderNode);
		label->centerUnder(centerUnderNode, 0.03f);
	}
	add(label);
	UINode *optionsPanel = new UINode(_frontend, title);
	{
		UIHBoxLayout *hlayout = new UIHBoxLayout(0.02f);
		optionsPanel->setLayout(hlayout);
		{
			UINodeButtonText *option1Text = new UINodeButtonText(_frontend, option1);
			option1Text->addListener(UINodeListenerPtr(option1Listener));
			option1Text->setFont(getFont(HUGE_FONT));
			optionsPanel->add(option1Text);

			UINodeButtonText *option2Text = new UINodeButtonText(_frontend, option2);
			option2Text->addListener(UINodeListenerPtr(option2Listener));
			option2Text->setFont(getFont(HUGE_FONT));
			optionsPanel->add(option2Text);
		}
	}
	add(optionsPanel);
	optionsPanel->centerUnder(label);
	return optionsPanel;
}
