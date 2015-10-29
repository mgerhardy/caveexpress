#include "IUIMapEditorWindow.h"
#include "ui/nodes/IUINodeEntitySelector.h"
#include "ui/nodes/IUINodeSpriteSelector.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeCheckbox.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSlider.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeSpinner.h"
#include "ui/nodes/UINodeTextInput.h"
#include "ui/nodes/UINodeButtonText.h"
#include "ui/layouts/UIVBoxLayout.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/layouts/UIFillLayout.h"
#include "ui/windows/listener/OpenWindowListener.h"
#include "common/MapManager.h"
#include "common/ConfigManager.h"
#include "common/TextureDefinition.h"
#include "miniracer/client/ui/windows/UIMapWindow.h"
#include "ui/windows/UISettingsWindow.h"
#include "ui/windows/mapeditor/QuitEditorListener.h"
#include "ui/nodes/IUINodeMapEditor.h"
#include "ui/nodes/UINodeMapEditorSelectedItem.h"
#include "ui/nodes/UINodeMapStringSelector.h"

#include "mapeditor/ChangeThemeListener.h"
#include "mapeditor/SaveListener.h"
#include "mapeditor/LoadListener.h"
#include "mapeditor/LoadListListener.h"
#include "mapeditor/NameListener.h"
#include "mapeditor/NewListener.h"
#include "mapeditor/UndoListener.h"
#include "mapeditor/RedoListener.h"
#include "mapeditor/SpriteSelectionListener.h"
#include "mapeditor/EntitySelectionListener.h"
#include "mapeditor/BooleanSettingListener.h"
#include "mapeditor/LayerListener.h"

IUIMapEditorWindow::IUIMapEditorWindow (IFrontend *frontend, IUINodeMapEditor* editor, IUINodeSpriteSelector* spriteSelector, IUINodeEntitySelector* entitySelector) :
		UIWindow(UI_WINDOW_EDITOR, frontend, WINDOW_FLAG_ROOT), _spritesNode(spriteSelector), _emitterNode(entitySelector), _selectedItemNode(nullptr), _mapEditor(editor)
{
}

void IUIMapEditorWindow::init(IMapManager& mapManager) {
	setBackgroundColor(colorDark);

	int cols = 6;
	int rows = 6;

	if (isSmallScreen()) {
		cols = 3;
		rows = 3;
	}

	_selectedItemNode = new UINodeMapEditorSelectedItem(_frontend);

	_spritesNode->setDimensions(cols, rows);
	_spritesNode->setId("spritesNode");
	_spritesNode->addSprites(ThemeTypes::ICE);
	_spritesNode->addListener(
			UINodeListenerPtr(new SpriteSelectionListener(_mapEditor, _spritesNode, _selectedItemNode)));

	_emitterNode->setDimensions(cols, 2);
	_emitterNode->setId("emitterNode");
	_emitterNode->addEntities(ThemeTypes::ICE);
	_emitterNode->addListener(
			UINodeListenerPtr(new EntitySelectionListener(_mapEditor, _emitterNode, _selectedItemNode)));

	UINode *left = new UINode(_frontend, "left");
	left->setLayout(new UIVBoxLayout());
	left->add(_spritesNode);
	left->add(_emitterNode);
	_selectedItemNode->setSize(_emitterNode->getWidth(), _emitterNode->getWidth());
	left->add(_selectedItemNode);
	left->add(createLayers());

	UINodeMapStringSelector *mapListNode = new UINodeMapStringSelector(_frontend, mapManager, 30);
	UINode *buttons = createButtons(mapManager, mapListNode);
	UINode *settings = createSettings();
	settings->setAlignment(NODE_ALIGN_BOTTOM);

	UINode *editorControls = new UINode(_frontend, "editorControls");
	editorControls->setLayout(new UIHBoxLayout());
	editorControls->add(left);

	UINode *mainPanel = new UINode(_frontend, "mainPanel");
	mainPanel->setLayout(new UIVBoxLayout());
	mainPanel->add(buttons);
	mainPanel->add(editorControls);

	add(mainPanel);
	add(settings);
	_mapEditor->setPos(left->getWidth(), buttons->getHeight());
	_mapEditor->setSize(1.0f - left->getWidth(), 1.0f - buttons->getHeight() - settings->getHeight());
	add(_mapEditor);
	add(mapListNode);
}

IUIMapEditorWindow::~IUIMapEditorWindow ()
{
}

UINode *IUIMapEditorWindow::createLayers ()
{
	const float checkBoxWidth = 36.0f / _frontend->getWidth();
	const float checkBoxHeight = 36.0f / _frontend->getHeight();
	UINode *layers = new UINode(_frontend, "layers");
	layers->setLayout(new UIVBoxLayout());
	for (int layer = LAYER_EMITTER; layer != LAYER_NONE; --layer) {
		UINodeCheckbox *layerCheckbox = new UINodeCheckbox(_frontend, "layer" + string::toString(layer));
		layerCheckbox->setSelected(true);
		layerCheckbox->setSize(checkBoxWidth, checkBoxHeight);
		layerCheckbox->setLabel(MapEditorLayerNames[layer]);
		layerCheckbox->setFont(getFont(), colorWhite);
		layerCheckbox->addListener(UINodeListenerPtr(new LayerListener(_mapEditor, static_cast<MapEditorLayer>(layer))));
		layers->add(layerCheckbox);
	}
	return layers;
}

UINode *IUIMapEditorWindow::createSettings ()
{
	UINode *settingsNode = new UINode(_frontend, "settings-node");
	UIHBoxLayout *hlayout = new UIHBoxLayout(6 / static_cast<float>(_frontend->getHeight()));
	settingsNode->setLayout(hlayout);

	UINodeTextInput *fileNode = new UINodeTextInput(_frontend, "", 10);
	fileNode->setBackgroundColor(colorWhite);
	fileNode->addListener(UINodeListenerPtr(new NameListener(_mapEditor, fileNode, true)));
	settingsNode->add(new UINodeSetting(_frontend, tr("File"), fileNode));

	UINodeTextInput *nameNode = new UINodeTextInput(_frontend, "", 14);
	nameNode->setBackgroundColor(colorWhite);
	nameNode->addListener(UINodeListenerPtr(new NameListener(_mapEditor, nameNode, false)));
	settingsNode->add(new UINodeSetting(_frontend, tr("Name"), nameNode));

	return settingsNode;
}

UINode *IUIMapEditorWindow::createButtons (IMapManager& mapManager,
		UINodeMapStringSelector *mapListNode)
{
	UINode *buttonsNode = new UINode(_frontend, "buttons-panel");
	buttonsNode->setLayout(new UIHBoxLayout());

	UINodeButton *newNode = new UINodeButtonText(_frontend, tr("New"));
	newNode->addListener(UINodeListenerPtr(new NewListener(_mapEditor)));
	buttonsNode->add(newNode);

	UINodeButton *loadNode = new UINodeButtonText(_frontend, tr("Load"));
	loadNode->addListener(UINodeListenerPtr(new LoadListener(loadNode, mapListNode)));
	buttonsNode->add(loadNode);
	mapListNode->setPos(loadNode->getLeft(), loadNode->getTop());
	mapListNode->addListener(UINodeListenerPtr(new LoadListListener(_mapEditor, loadNode, mapListNode)));

	UINodeButton *saveNode = new UINodeButtonText(_frontend, tr("Save"));
	saveNode->addListener(UINodeListenerPtr(new SaveListener(_mapEditor)));
	buttonsNode->add(saveNode);

	UINodeSaveButton *saveAndGoNode = new UINodeSaveButton(_frontend);
	saveAndGoNode->addListener(UINodeListenerPtr(new SaveListener(_mapEditor, saveAndGoNode, true)));
	buttonsNode->add(saveAndGoNode);

	UINodeButton *undoNode = new UINodeButtonText(_frontend, tr("Undo"));
	undoNode->addListener(UINodeListenerPtr(new UndoListener(_mapEditor)));
	buttonsNode->add(undoNode);

	UINodeButton *redoNode = new UINodeButtonText(_frontend, tr("Redo"));
	redoNode->addListener(UINodeListenerPtr(new RedoListener(_mapEditor)));
	buttonsNode->add(redoNode);

	UINodeButton *mapOptionsButton = new UINodeButtonText(_frontend, tr("Map Options"));
	mapOptionsButton->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_MAPEDITOR_OPTIONS)));
	buttonsNode->add(mapOptionsButton);

	UINodeButton *helpNode = new UINodeButtonText(_frontend, tr("Help"));
	helpNode->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_MAPEDITOR_HELP)));
	buttonsNode->add(helpNode);

	UINodeButton *settingsNode = new UINodeButtonText(_frontend, tr("Settings"));
	settingsNode->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_SETTINGS)));
	buttonsNode->add(settingsNode);

	UINodeButton *quitNode = new UINodeButtonText(_frontend, tr("Quit"));
	quitNode->addListener(UINodeListenerPtr(new QuitEditorListener(_mapEditor)));
	buttonsNode->add(quitNode);

	return buttonsNode;
}

bool IUIMapEditorWindow::onPush ()
{
	const bool ret = UIWindow::onPush();
	_mapEditor->loadLast();
	return ret;
}
