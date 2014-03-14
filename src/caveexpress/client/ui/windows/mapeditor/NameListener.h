#pragma once

class NameListener: public UINodeListener, public IMapEditorListener {
private:
	UINodeMapEditor *_mapEditor;
	UINodeTextInput *_nameNode;
	bool _fileName;
	bool _blocked;
public:
	NameListener (UINodeMapEditor *mapEditor, UINodeTextInput *nameNode, bool fileName) :
			_mapEditor(mapEditor), _nameNode(nameNode), _fileName(fileName), _blocked(false)
	{
		_mapEditor->addEditorListener(this);
	}

	void onValueChanged () override
	{
		_blocked = true;
		if (_fileName)
			_mapEditor->setFileName(_nameNode->getValue());
		else
			_mapEditor->setMapName(_nameNode->getValue());
		_blocked = false;
	}

	void onFileNameChange (const std::string& oldName, const std::string& newName) override
	{
		if (_blocked)
			return;

		if (!_fileName)
			return;
		_nameNode->setValue(newName);
	}

	void onMapNameChange (const std::string& oldName, const std::string& newName) override
	{
		if (_blocked)
			return;

		if (_fileName)
			return;
		_nameNode->setValue(newName);
	}
};
