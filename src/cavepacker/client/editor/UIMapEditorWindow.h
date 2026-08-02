#pragma once

#include "ui/editor/UIMapEditorWindow.h"

namespace cavepacker {

class UIMapEditorWindow: public ::UIMapEditorWindow {
public:
	UIMapEditorWindow (IFrontend* frontend, IMapManager& mapManager);
};

}
