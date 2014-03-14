#include "UIGameFinishedWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/common/Commands.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"

UIGameFinishedWindow::UIGameFinishedWindow (IFrontend *frontend) :
		UIWindow(UI_WINDOW_GAMEFINISHED, frontend)
{
	_musicFile = "music-win";
	setInactiveAfterPush();

	UINodeBackground *background = new UINodeBackground(frontend, "", false);
	if (System.hasTouch() && !wantBackButton())
		background->setOnActivate(CMD_UI_POP);
	add(background);

	UINodeLabel *won = new UINodeLabel(frontend, tr("You won!"));
	won->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	won->setFont(LARGE_FONT);
	won->setColor(colorWhite);
	add(won);

	// TODO:

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}
