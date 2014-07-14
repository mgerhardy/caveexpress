#include "UIMapEditorWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeCheckbox.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/client/ui/nodes/UINodeSlider.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeSpinner.h"
#include "engine/client/ui/nodes/UINodeTextInput.h"
#include "engine/client/ui/nodes/UINodeButtonText.h"
#include "engine/client/ui/layouts/UIVBoxLayout.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"
#include "engine/client/ui/layouts/UIFillLayout.h"
#include "engine/client/ui/windows/listener/OpenWindowListener.h"
#include "engine/common/MapManager.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/TextureDefinition.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "caveexpress/client/ui/windows/UISettingsWindow.h"
#include "caveexpress/client/ui/windows/mapeditor/QuitEditorListener.h"
#include "caveexpress/client/ui/nodes/UINodeMapEditor.h"
#include "caveexpress/client/ui/nodes/UINodeSpriteSelector.h"
#include "caveexpress/client/ui/nodes/UINodeEntitySelector.h"
#include "caveexpress/client/ui/nodes/UINodeMapEditorSelectedItem.h"
#include "caveexpress/client/ui/nodes/UINodeMapStringSelector.h"
#include "caveexpress/shared/constants/Commands.h"

#include "mapeditor/SaveListener.h"
#include "mapeditor/LoadListener.h"
#include "mapeditor/LoadListListener.h"
#include "mapeditor/WaterHeightListener.h"
#include "mapeditor/NameListener.h"
#include "mapeditor/NewListener.h"
#include "mapeditor/UndoListener.h"
#include "mapeditor/RedoListener.h"
#include "mapeditor/ChangeThemeListener.h"
#include "mapeditor/AutoGenerateListener.h"
#include "mapeditor/SpriteSelectionListener.h"
#include "mapeditor/EntitySelectionListener.h"
#include "mapeditor/BooleanSettingListener.h"
#include "mapeditor/LayerListener.h"

class UINodeSetting: public UINode {
public:
	UINodeSetting (IFrontend* frontend, const std::string& label, UINode* node, const std::string& tooltip = "") :
			UINode(frontend, label) {
		setTooltip(tooltip);
		setLayout(new UIVBoxLayout(0.01f, true));
		UINodeLabel *labelNode = new UINodeLabel(_frontend, label);
		add(labelNode);
		add(node);
	}
};

UIMapEditorWindow::UIMapEditorWindow (IFrontend *frontend, IMapManager& mapManager) :
		UIWindow(UI_WINDOW_EDITOR, frontend, WINDOW_FLAG_ROOT)
{
	setBackgroundColor(colorDark);
	_mapEditor = new UINodeMapEditor(_frontend, mapManager);

	int cols = 6;
	int rows = 6;

	if (isSmallScreen()) {
		cols = 3;
		rows = 3;
	}

	_selectedItemNode = new UINodeMapEditorSelectedItem(_frontend);

	_spritesNode = new UINodeSpriteSelector(_frontend, cols, rows);
	_spritesNode->setId("spritesNode");
	_spritesNode->addSprites(ThemeTypes::ICE);
	_spritesNode->addListener(
			UINodeListenerPtr(new SpriteSelectionListener(_mapEditor, _spritesNode, _selectedItemNode)));

	_emitterNode = new UINodeEntitySelector(_frontend, cols, 2);
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

bool UIMapEditorWindow::onPush ()
{
	const bool ret = UIWindow::onPush();
	_mapEditor->loadLast();
	return ret;
}
