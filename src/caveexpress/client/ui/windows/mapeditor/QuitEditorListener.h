#pragma once

#include "ui/UI.h"
#include "common/Commands.h"
#include "caveexpress/client/ui/nodes/UINodeMapEditor.h"

namespace caveexpress {

class QuitEditorListener: public UINodeListener {
private:
	UINodeMapEditor *_editor;
public:
	QuitEditorListener(UINodeMapEditor *editor) :
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

}
