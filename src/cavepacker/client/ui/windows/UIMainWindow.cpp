#include "UIMainWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/windows/listener/QuitListener.h"
#include "engine/client/ui/nodes/UINodeMainBackground.h"
#include "engine/client/ui/layouts/UIVBoxLayout.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Version.h"
#include "engine/client/ui/windows/listener/OpenWindowListener.h"
#include "engine/client/ui/nodes/UINodeMainButton.h"

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

	UINodeMainButton *campaign = new UINodeMainButton(_frontend, tr("Maps"));
	campaign->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_CAMPAIGN_MAPS)));
	panel->add(campaign);

#if 0
	UINodeMainButton *settings = new UINodeMainButton(_frontend, tr("Settings"));
	settings->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_SETTINGS)));
	panel->add(settings);
#endif

	UINodeMainButton *twitter = new UINodeMainButton(_frontend, tr("Twitter"));
	twitter->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "https://twitter.com/MartinGerhardy")));
	panel->add(twitter);

	UINodeMainButton *facebook = new UINodeMainButton(_frontend, tr("Facebook"));
	facebook->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "https://facebook.com/" APPNAME)));
	panel->add(facebook);

	UINodeMainButton *quit = new UINodeMainButton(_frontend, tr("Quit"));
	quit->addListener(UINodeListenerPtr(new QuitListener()));
	panel->add(quit);

	add(panel);
}
