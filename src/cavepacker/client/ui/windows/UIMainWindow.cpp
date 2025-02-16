#include "UIMainWindow.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeButtonImage.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeGooglePlayButton.h"
#include "ui/windows/UIWindow.h"
#include "ui/windows/listener/QuitListener.h"
#include "ui/nodes/UINodeMainBackground.h"
#include "ui/layouts/UIVBoxLayout.h"
#include "common/ConfigManager.h"
#include "common/System.h"
#include "common/Application.h"
#include "ui/windows/listener/OpenWindowListener.h"
#include "ui/nodes/UINodeMainButton.h"

namespace cavepacker {

UIMainWindow::UIMainWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_MAIN, frontend, WINDOW_FLAG_ROOT)
{
	add(new UINodeMainBackground(frontend, false));
	const float padding = 10.0f / static_cast<float>(_frontend->getHeight());
	UINode *panel = new UINode(_frontend, "panelMain");
	UIVBoxLayout *layout = new UIVBoxLayout(padding, true);
	panel->setLayout(layout);
	panel->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	panel->setPadding(padding);

	UINodeMainButton *campaign = new UINodeMainButton(_frontend, tr("Start"));
	campaign->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_CAMPAIGN)));
	panel->add(campaign);

#ifndef NONETWORK
	if (Config.isNetwork()) {
		UINodeMainButton *multiplayer = new UINodeMainButton(_frontend, tr("Multiplayer"));
		multiplayer->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_MULTIPLAYER)));
		panel->add(multiplayer);
	}
#endif

	UINodeMainButton *settings = new UINodeMainButton(_frontend, tr("Settings"));
	settings->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_SETTINGS)));
	panel->add(settings);

	if (System.supportGooglePlay()) {
		UINodeButtonImage *googlePlay = new UINodeGooglePlayButton(_frontend);
		googlePlay->setPadding(padding);
		add(googlePlay);
	}

#ifndef STEAMLINK
	if (System.supportsUserContent()) {
		UINodeMainButton *editor = new UINodeMainButton(_frontend, tr("Editor"));
		editor->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_EDITOR)));
		panel->add(editor);
	}

	UINodeMainButton *homepage = new UINodeMainButton(_frontend, tr("Homepage"));
	homepage->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "http://caveproductions.org/")));
	panel->add(homepage);
#endif

#if 0
#ifdef __EMSCRIPTEN__
	UINodeMainButton *fullscreen = new UINodeMainButton(_frontend, tr("Fullscreen"));
	fullscreen->addListener(UINodeListenerPtr(new EmscriptenFullscreenListener()));
	panel->add(fullscreen);
#endif
#endif

	UINodeMainButton *quit = new UINodeMainButton(_frontend, tr("Quit"));
#ifdef __EMSCRIPTEN__
	quit->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "http://caveproductions.org/", false)));
#else
	quit->addListener(UINodeListenerPtr(new QuitListener()));
#endif
	panel->add(quit);

	add(panel);

	Application& app = Singleton<Application>::getInstance();
	UINodeLabel *versionLabel = new UINodeLabel(_frontend, app.getPackageName() + " " + app.getVersion());
	versionLabel->setAlignment(NODE_ALIGN_BOTTOM|NODE_ALIGN_RIGHT);
	versionLabel->setColor(colorWhite);
	versionLabel->setPadding(getScreenPadding());
	add(versionLabel);
}

}
