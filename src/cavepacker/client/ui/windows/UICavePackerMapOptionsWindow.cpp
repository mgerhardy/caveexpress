#include "UICavePackerMapOptionsWindow.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeBackToRootButton.h"
#include "ui/nodes/UINodeButtonText.h"
#include "common/ServiceProvider.h"
#include "common/Commands.h"
#include "network/INetwork.h"
#include "ui/nodes/UINodeSettingsBackground.h"

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
	if (_serviceProvider.getNetwork().isMultiplayer()) {
		_solve->setVisible(false);
	} else if (System.supportPayment()) {
		_solve->setVisible(System.hasItem("autosolve"));
	}
}
