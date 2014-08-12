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
	_solve = new UINodeButtonImage(frontend, "icon-solve");
	_solve->putUnder(_restartMap, 0.02f);
	_solve->setOnActivate(CMD_UI_POP ";solve");
	if (_backButton == nullptr)
		add(_solve);
	else
		addBefore(_backButton, _solve);
}

void UICavePackerMapOptionsWindow::onActive ()
{
	UIMapOptionsWindow::onActive();
	if (System.supportPayment()) {
		_solve->setVisible(System.hasItem("autosolve"));
	}
}
