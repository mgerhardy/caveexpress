#pragma once

namespace caveexpress {

class LayerListener: public UINodeListener {
private:
	UINodeMapEditor *_mapEditor;
	MapEditorLayer _layer;
public:
	LayerListener (UINodeMapEditor *mapEditor, MapEditorLayer layer) :
			_mapEditor(mapEditor), _layer(layer)
	{
	}

	void onClick () override
	{
		_mapEditor->toggleLayer(_layer);
	}
};

}
