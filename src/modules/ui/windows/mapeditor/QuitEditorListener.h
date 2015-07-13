#pragma once

#include "ui/UI.h"
#include "common/Commands.h"
#include "ui/nodes/IUINodeMapEditor.h"

class QuitEditorListener: public UINodeListener {
private:
	IUINodeMapEditor *_editor;
public:
	QuitEditorListener(IUINodeMapEditor *editor) :
			_editor(editor) {
	}

	void onClick() override {
		UIPopupCallbackPtr c(new UIPopupOkCommandCallback(CMD_QUIT));
		if (_editor->isDirty()) {
			UI::get().popup(tr("Quit without saving"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
		} else {
			UI::get().popup(tr("Quit"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
		}
	}
};
