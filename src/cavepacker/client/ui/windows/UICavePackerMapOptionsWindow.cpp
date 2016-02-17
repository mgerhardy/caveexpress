#include "UICavePackerMapOptionsWindow.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeBackToRootButton.h"
#include "ui/nodes/UINodeMainButton.h"
#include "ui/nodes/UINodeSettingsBackground.h"
#include "ui/layouts/UIVBoxLayout.h"
#include "service/ServiceProvider.h"
#include "common/Commands.h"
#include "listener/SolveListener.h"
#include "network/INetwork.h"

namespace cavepacker {

UICavePackerMapOptionsWindow::UICavePackerMapOptionsWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIMapOptionsWindow(frontend, serviceProvider)
{
	_solve = new UINode(frontend);
	_solve->setLayout(new UIVBoxLayout(0.02f));

	UINodeMainButton* button = new UINodeMainButton(frontend, tr("Solve"));
	button->setOnActivate(CMD_UI_POP ";solve");

	UINodeMainButton* skipButton = new UINodeMainButton(frontend, tr("Skip"));
	skipButton->setOnActivate(CMD_UI_POP ";finish");
	skipButton->putUnder(_restartMap, 0.02f);
	if (_backButton == nullptr) {
		_panel->add(skipButton);
	} else {
		_panel->addBefore(_backButton, skipButton);
	}

	UINodeSlider* autoSolveSlider = new UINodeSlider(frontend, 10.0f, 1000.0f, 10.0f);
	autoSolveSlider->setSize(0.1f, 0.05f);
	autoSolveSlider->addListener(UINodeListenerPtr(new SolveListener(autoSolveSlider, "solvestepmillis")));
	autoSolveSlider->setId("autosolveslideroptions");

	_solve->add(button);
	_solve->add(autoSolveSlider);
	_solve->putUnder(skipButton, 0.02f);

	if (_backButton == nullptr) {
		_panel->add(_solve);
	} else {
		_panel->addBefore(_backButton, _solve);
	}
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

}
