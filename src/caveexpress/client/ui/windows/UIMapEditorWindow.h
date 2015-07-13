#pragma once

#include "ui/windows/IUIMapEditorWindow.h"

namespace caveexpress {

class UIMapEditorWindow: public IUIMapEditorWindow {
	friend class AutoGenerateListener;
protected:
	virtual UINode *createSettings () override;
	virtual UINode *createLayers () override;
	virtual UINode *createButtons (IMapManager& mapManager, UINodeMapStringSelector *mapListNode) override;

public:
	UIMapEditorWindow (IFrontend *frontend, IMapManager& mapManager, IUINodeMapEditor* editor);
	virtual ~UIMapEditorWindow ();
};

}
