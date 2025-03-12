#pragma once

#include "ui/windows/IUIMapEditorWindow.h"

namespace caveexpress {

class UIMapEditorWindow: public IUIMapEditorWindow {
	friend class AutoGenerateListener;
	using Super = IUIMapEditorWindow;
protected:
	virtual UINode *createSettings () override;
	virtual UINode *createButtons (IMapManager& mapManager, UINodeMapStringSelector *mapListNode) override;

public:
	UIMapEditorWindow (IFrontend *frontend, IMapManager& mapManager, IUINodeMapEditor* editor);
	virtual ~UIMapEditorWindow ();
};

}
