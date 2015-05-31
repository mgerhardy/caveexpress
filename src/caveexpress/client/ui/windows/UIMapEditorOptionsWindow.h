#pragma once

#include "client/ui/windows/UIWindow.h"

class UINodeMapEditor;

class UIMapEditorOptionsWindow: public UIWindow {
protected:
	UINodeMapEditor* _mapEditor;
	UINode* createSection (UINode* parent, const std::string& title);
	UINode* createGeneralOptions (UINode* vbox);
	UINode* createWinConditions (UINode* vbox);
	UINode* createWaterOptions (UINode* vbox);
public:
	UIMapEditorOptionsWindow (IFrontend* frontend, UINodeMapEditor* mapEditor);
	virtual ~UIMapEditorOptionsWindow ();
};
