#pragma once

class LayerListener: public UINodeListener {
private:
	IUINodeMapEditor *_mapEditor;
	MapEditorLayer _layer;
public:
	LayerListener (IUINodeMapEditor *mapEditor, MapEditorLayer layer) :
			_mapEditor(mapEditor), _layer(layer)
	{
	}

	void onClick () override
	{
		_mapEditor->toggleLayer(_layer);
	}
};
