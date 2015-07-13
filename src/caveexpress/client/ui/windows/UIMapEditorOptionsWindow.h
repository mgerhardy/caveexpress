#pragma once

#include "ui/windows/IUIMapEditorOptionsWindow.h"

namespace caveexpress {

class UINodeMapEditor;

class UIMapEditorOptionsWindow: public IUIMapEditorOptionsWindow {
protected:
	virtual void fillGeneralOptions (UINode* hbox) override;
	virtual void fillWinConditions (UINode* hbox) override;
	virtual UINode* createGeneralOptions (UINode* vbox) override;
	UINode* createWaterOptions (UINode* vbox);
public:
	UIMapEditorOptionsWindow (IFrontend* frontend, UINodeMapEditor* mapEditor);
	virtual ~UIMapEditorOptionsWindow ();
};

}
