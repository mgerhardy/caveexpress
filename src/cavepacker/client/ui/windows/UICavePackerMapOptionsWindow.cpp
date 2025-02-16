#include "UICavePackerMapOptionsWindow.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeMainButton.h"
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

	_skipButton = new UINodeMainButton(frontend, tr("Skip"));
	_skipButton->setOnActivate(CMD_UI_POP ";finish");
	_skipButton->putUnder(_restartMap, 0.02f);
	if (_backButton == nullptr) {
		_panel->add(_skipButton);
	} else {
		_panel->addBefore(_backButton, _skipButton);
	}

	UINodeSlider* autoSolveSlider = new UINodeSlider(frontend, 10.0f, 1000.0f, 10.0f);
	autoSolveSlider->setSize(0.1f, 0.05f);
	autoSolveSlider->addListener(UINodeListenerPtr(new SolveListener(autoSolveSlider, "solvestepmillis")));
	autoSolveSlider->setId("autosolveslideroptions");

	_solve->add(button);
	_solve->add(autoSolveSlider);
	_solve->putUnder(_skipButton, 0.02f);

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
		_skipButton->setVisible(false);
		return;
	}
	_solve->setVisible(true);
	_skipButton->setVisible(true);
}

}
