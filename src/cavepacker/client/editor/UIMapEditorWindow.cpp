#include "UIMapEditorWindow.h"
#include "cavepacker/client/editor/MapEditorDocument.h"
#include <memory>

namespace cavepacker {

UIMapEditorWindow::UIMapEditorWindow (IFrontend* frontend, IMapManager& mapManager) :
		::UIMapEditorWindow(frontend, std::unique_ptr<IMapEditorDocument>(new MapEditorDocument(mapManager)))
{
}

}
