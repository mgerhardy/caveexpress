#pragma once

#include "engine/client/ui/UI.h"
#include "caveexpress/client/ui/nodes/UINodeMapEditor.h"
#include "engine/client/ui/windows/listener/QuitPopupCallback.h"

class QuitEditorListener: public UINodeListener {
private:
	UINodeMapEditor *_editor;
public:
	QuitEditorListener(UINodeMapEditor *editor) :
			_editor(editor) {
	}

	void onClick() override {
		UIPopupCallbackPtr c(new QuitPopupCallback());
		if (_editor->isDirty()) {
			UI::get().popup(tr("Quit without saving"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
		} else {
			UI::get().popup(tr("Quit"), UIPOPUP_OK | UIPOPUP_CANCEL, c);
		}
	}
};
