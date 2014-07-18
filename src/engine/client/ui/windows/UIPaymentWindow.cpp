#include "UIPaymentWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/client/ui/nodes/UINodeBackground.h"
#include "engine/client/ui/nodes/UINodePaymentSelector.h"

UIPaymentWindow::UIPaymentWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_PAYMENT, frontend, WINDOW_FLAG_MODAL)
{
	UINode* background = new UINodeBackground(frontend, tr("Extras"));
	add(background);

	UINodePaymentSelector *payments = new UINodePaymentSelector(frontend, 2);
	add(payments);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}
