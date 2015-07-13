#pragma once

#include "ui/windows/UIWindow.h"
#include "ui/nodes/UINodeCheckbox.h"
#include "ui/nodes/UINodeSpinner.h"
#include "ui/nodes/UINodeSlider.h"

class IUINodeMapEditor;

class UINodeSettingsCheckbox : public UINodeCheckbox {
public:
	UINodeSettingsCheckbox (UINode* parent, IUINodeMapEditor* mapEditor, const std::string& label, const std::string& setting, const std::string& tooltip = "");
};

class UINodeSettingsSpinner : public UINodeSpinner {
public:
	UINodeSettingsSpinner (UINode* parent, int min, int max, int stepWidth, IUINodeMapEditor* mapEditor,
			const std::string& label, const std::string& setting, const std::string& tooltip = "");
};

class UINodeSettingsSlider : public UINodeSlider {
public:
	UINodeSettingsSlider (UINode* parent, float min, float max, float stepWidth, IUINodeMapEditor* mapEditor, const std::string& label, const std::string& setting, const std::string& tooltip = "");
};

class IUIMapEditorOptionsWindow: public UIWindow {
protected:
	IUINodeMapEditor* _mapEditor;
	UINode* _vbox;
	UINode* createSection (UINode* parent, const std::string& title);
	virtual UINode* createGeneralOptions (UINode* vbox);
	virtual UINode* createWinConditions (UINode* vbox);

	virtual void fillGeneralOptions (UINode* hbox);
	virtual void fillWinConditions (UINode* hbox);
public:
	IUIMapEditorOptionsWindow (IFrontend* frontend, IUINodeMapEditor* mapEditor);
	virtual ~IUIMapEditorOptionsWindow ();
	void init();
};
