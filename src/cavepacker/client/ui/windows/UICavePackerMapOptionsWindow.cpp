#include "UICavePackerMapOptionsWindow.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeBackToRootButton.h"
#include "engine/client/ui/nodes/UINodeButtonText.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/Commands.h"
#include "engine/common/network/INetwork.h"
#include "engine/client/ui/nodes/UINodeSettingsBackground.h"

UICavePackerMapOptionsWindow::UICavePackerMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIMapOptionsWindow(frontend, serviceProvider)
{
	UINodeButtonImage *solve = new UINodeButtonImage(frontend, "icon-reload");
	solve->setId("restart-map");
	solve->putUnder(_restartMap, 0.02f);
	solve->setOnActivate(CMD_UI_POP ";solve");
	add(solve);
}
