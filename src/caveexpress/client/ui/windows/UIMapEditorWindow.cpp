#include "UIMapEditorWindow.h"

#include "ui/UI.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeCheckbox.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSlider.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeSpinner.h"
#include "ui/nodes/UINodeTextInput.h"
#include "ui/nodes/UINodeButtonText.h"
#include "ui/nodes/IUINodeMapEditor.h"
#include "ui/nodes/UINodeMapEditorSelectedItem.h"
#include "ui/nodes/UINodeMapStringSelector.h"

#include "ui/layouts/UIVBoxLayout.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/layouts/UIFillLayout.h"

#include "ui/windows/UISettingsWindow.h"
#include "ui/windows/listener/OpenWindowListener.h"
#include "ui/windows/mapeditor/QuitEditorListener.h"
#include "ui/windows/mapeditor/LayerListener.h"
#include "ui/windows/mapeditor/ChangeThemeListener.h"
#include "ui/windows/mapeditor/SaveListener.h"
#include "ui/windows/mapeditor/LoadListener.h"
#include "ui/windows/mapeditor/LoadListListener.h"
#include "ui/windows/mapeditor/NameListener.h"
#include "ui/windows/mapeditor/NewListener.h"
#include "ui/windows/mapeditor/UndoListener.h"
#include "ui/windows/mapeditor/RedoListener.h"
#include "ui/windows/mapeditor/SpriteSelectionListener.h"
#include "ui/windows/mapeditor/EntitySelectionListener.h"
#include "ui/windows/mapeditor/BooleanSettingListener.h"

#include "common/MapManager.h"
#include "common/ConfigManager.h"
#include "common/TextureDefinition.h"

#include "caveexpress/client/ui/nodes/UINodeEntitySelector.h"
#include "caveexpress/client/ui/nodes/UINodeSpriteSelector.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "mapeditor/WaterHeightListener.h"
#include "mapeditor/AutoGenerateListener.h"

namespace caveexpress {

UIMapEditorWindow::UIMapEditorWindow (IFrontend *frontend, IMapManager& mapManager, IUINodeMapEditor* editor) :
		IUIMapEditorWindow(frontend, editor, new UINodeSpriteSelector(frontend), new UINodeEntitySelector(frontend))
{
	init(mapManager);
}

UIMapEditorWindow::~UIMapEditorWindow ()
{
}

UINode *UIMapEditorWindow::createLayers ()
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

UINode *UIMapEditorWindow::createSettings ()
{
	UINode *settingsNode = new UINode(_frontend, "settings-node");
	UIHBoxLayout *hlayout = new UIHBoxLayout(6 / static_cast<float>(_frontend->getHeight()));
	settingsNode->setLayout(hlayout);

	UINodeTextInput *fileNode = new UINodeTextInput(_frontend, "", 10);
	fileNode->setId("filename");
	fileNode->setBackgroundColor(colorWhite);
	fileNode->addListener(UINodeListenerPtr(new NameListener(_mapEditor, fileNode, true)));
	settingsNode->add(new UINodeSetting(_frontend, tr("File"), fileNode));

	UINodeTextInput *nameNode = new UINodeTextInput(_frontend, "", 14);
	nameNode->setBackgroundColor(colorWhite);
	nameNode->addListener(UINodeListenerPtr(new NameListener(_mapEditor, nameNode, false)));
	settingsNode->add(new UINodeSetting(_frontend, tr("Name"), nameNode));

	const float height = 0.025f;
	const float sliderWidth = 0.1f;
	UINodeSlider *waterHeightNode = new UINodeSlider(_frontend, 0.0f, 0.0f, 0.1f);
	waterHeightNode->setId("water-height-node");
	waterHeightNode->setSize(sliderWidth, height);
	waterHeightNode->addListener(UINodeListenerPtr(new WaterHeightListener(_mapEditor, waterHeightNode)));
	settingsNode->add(new UINodeSetting(_frontend, tr("Waterheight"), waterHeightNode));

	return settingsNode;
}

UINode *UIMapEditorWindow::createButtons (IMapManager& mapManager,
		UINodeMapStringSelector *mapListNode)
{
	UINode *buttonsNode = new UINode(_frontend, "buttons-panel");
	buttonsNode->setLayout(new UIHBoxLayout());

	UINodeButton *iceNode = new UINodeButtonText(_frontend, tr("Ice"));
	iceNode->addListener(UINodeListenerPtr(new ChangeThemeListener(_mapEditor, _spritesNode, _emitterNode, _selectedItemNode,
							ThemeTypes::ICE)));
	buttonsNode->add(iceNode);

	UINodeButton *rockNode = new UINodeButtonText(_frontend, tr("Rock"));
	rockNode->addListener(UINodeListenerPtr(new ChangeThemeListener(_mapEditor, _spritesNode, _emitterNode, _selectedItemNode,
							ThemeTypes::ROCK)));
	buttonsNode->add(rockNode);

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

	UINodeButton *autogenerateNode = new UINodeButtonText(_frontend, tr("Auto"));
	autogenerateNode->addListener(UINodeListenerPtr(new AutoGenerateListener(_mapEditor, _spritesNode)));
	buttonsNode->add(autogenerateNode);

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

}
