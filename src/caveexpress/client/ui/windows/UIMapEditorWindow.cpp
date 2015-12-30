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
		Super(frontend, editor, new UINodeSpriteSelector(frontend), new UINodeEntitySelector(frontend))
{
	init(mapManager);
}

UIMapEditorWindow::~UIMapEditorWindow ()
{
}

UINode *UIMapEditorWindow::createSettings ()
{
	UINode *settingsNode = Super::createSettings();

	const float height = 0.025f;
	const float sliderWidth = 0.1f;
	UINodeSlider *waterHeightNode = new UINodeSlider(_frontend, 0.0f, 0.0f, 0.1f);
	waterHeightNode->setId("water-height-node");
	waterHeightNode->setSize(sliderWidth, height);
	waterHeightNode->addListener(UINodeListenerPtr(new WaterHeightListener(_mapEditor, waterHeightNode)));
	settingsNode->add(new UINodeSetting(_frontend, tr("Waterheight"), waterHeightNode));

	return settingsNode;
}

UINode *UIMapEditorWindow::createButtons (IMapManager& mapManager, UINodeMapStringSelector *mapListNode)
{
	UINode *buttonsNode = Super::createButtons(mapManager, mapListNode);

	UINodeButton *autogenerateNode = new UINodeButtonText(_frontend, tr("Auto"));
	autogenerateNode->addListener(UINodeListenerPtr(new AutoGenerateListener(_mapEditor, _spritesNode)));
	buttonsNode->addFront(autogenerateNode);

	UINodeButton *rockNode = new UINodeButtonText(_frontend, tr("Rock"));
	rockNode->addListener(UINodeListenerPtr(new ChangeThemeListener(_mapEditor, _spritesNode, _emitterNode, _selectedItemNode, ThemeTypes::ROCK)));
	buttonsNode->addFront(rockNode);

	UINodeButton *iceNode = new UINodeButtonText(_frontend, tr("Ice"));
	iceNode->addListener(UINodeListenerPtr(new ChangeThemeListener(_mapEditor, _spritesNode, _emitterNode, _selectedItemNode, ThemeTypes::ICE)));
	buttonsNode->addFront(iceNode);

	return buttonsNode;
}

}
