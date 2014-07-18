#include "UIMapEditorOptionsWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodeBackground.h"
#include "caveexpress/client/ui/nodes/UINodeMapEditor.h"

#include "engine/client/ui/nodes/UINodeBackButton.h"
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
#include "engine/client/ui/nodes/UINodeMapStringSelector.h"
#include "caveexpress/shared/constants/Commands.h"

#include "mapeditor/IntSettingsListener.h"
#include "mapeditor/FloatSettingsListener.h"
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
#include "mapeditor/ToggleGridListener.h"
#include "mapeditor/EntitySelectionListener.h"
#include "mapeditor/BooleanSettingListener.h"
#include "mapeditor/LayerListener.h"

namespace {
const float height = 0.025f;
const float spinnerWidth = 0.05f;
const float sliderWidth = 0.1f;
}

class UINodeSetting: public UINode {
public:
	UINodeSetting (IFrontend* frontend, const std::string& label, UINode* node, const std::string& tooltip = "") :
			UINode(frontend, label) {
		setTooltip(tooltip);
		setLayout(new UIVBoxLayout(0.01f, false));
		UINodeLabel *labelNode = new UINodeLabel(_frontend, label);
		add(labelNode);
		add(node);
	}
};

class UINodeSettingsCheckbox : public UINodeCheckbox {
public:
	UINodeSettingsCheckbox (UINode* parent, UINodeMapEditor* mapEditor, const std::string& label, const std::string& setting, const std::string& tooltip = "") :
			UINodeCheckbox(parent->getFrontend(), label) {
		UINodeListener* listener = new BooleanSettingListener(mapEditor, this, setting);
		const float checkBoxWidth = 36.0f / _frontend->getWidth();
		const float checkBoxHeight = 36.0f / _frontend->getHeight();
		setSize(checkBoxWidth, checkBoxHeight);
		setFont(getFont(), colorBlack);
		setTooltip(tooltip);
		addListener(UINodeListenerPtr(listener));
		parent->add(new UINodeSetting(_frontend, label, this));
	}
};

class UINodeSettingsSpinner : public UINodeSpinner {
public:
	UINodeSettingsSpinner (UINode* parent, int min, int max, int stepWidth, UINodeMapEditor* mapEditor,
			const std::string& label, const std::string& setting, const std::string& tooltip = "") :
			UINodeSpinner(parent->getFrontend(), min, max, stepWidth) {
		UINodeListener* listener = new IntSettingsListener(mapEditor, this, setting);
		setId(label);
		setBackgroundColor(colorWhite);
		setSize(spinnerWidth, height);
		setTooltip(tooltip);
		addListener(UINodeListenerPtr(listener));
		parent->add(new UINodeSetting(_frontend, label, this));
	}
};

class UINodeSettingsSlider : public UINodeSlider {
public:
	UINodeSettingsSlider (UINode* parent, float min, float max, float stepWidth, UINodeMapEditor* mapEditor, const std::string& label, const std::string& setting, const std::string& tooltip = "") :
			UINodeSlider(parent->getFrontend(), min, max, stepWidth) {
		UINodeListener* listener = new FloatSettingsListener(mapEditor, this, setting);
		setId(label);
		setSize(sliderWidth, height);
		setTooltip(tooltip);
		addListener(UINodeListenerPtr(listener));
		parent->add(new UINodeSetting(_frontend, label, this));
	}
};


UIMapEditorOptionsWindow::UIMapEditorOptionsWindow (IFrontend *frontend, UINodeMapEditor* mapEditor) :
		UIWindow(UI_WINDOW_MAPEDITOR_OPTIONS, frontend, WINDOW_FLAG_MODAL), _mapEditor(mapEditor)
{
	UINode *background = new UINode(_frontend, "background");
	background->setBackgroundColor(colorDark);
	background->setSize(1.0f, 1.0f);

	UINode *vbox = new UINode(_frontend, "vbox");
	vbox->setLayout(new UIVBoxLayout(0.02f, true, NODE_ALIGN_CENTER));

	createGeneralOptions(vbox);
	createWaterOptions(vbox);
	createWinConditions(vbox);

	background->add(vbox);
	add(background);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

UIMapEditorOptionsWindow::~UIMapEditorOptionsWindow ()
{
}

UINode* UIMapEditorOptionsWindow::createSection (UINode* parent, const std::string& title)
{
	UINodeLabel *label = new UINodeLabel(_frontend, title);
	label->setFont(HUGE_FONT);
	label->setColor(colorWhite);
	label->setAlignment(NODE_ALIGN_CENTER);
	parent->add(label);
	UINode *optionsPanel = new UINode(_frontend, title);
	UIHBoxLayout *hlayout = new UIHBoxLayout(0.02f);
	optionsPanel->setLayout(hlayout);
	optionsPanel->setAlignment(NODE_ALIGN_CENTER);
	return optionsPanel;
}

UINode* UIMapEditorOptionsWindow::createWaterOptions (UINode* vbox)
{
	UINode *hbox = createSection(vbox, tr("Water"));
	UINodeSlider *waterHeightNode = new UINodeSlider(_frontend, 0.0f, 0.0f, 0.1f);
	waterHeightNode->setId("water-height-node");
	waterHeightNode->setSize(sliderWidth, height);
	waterHeightNode->addListener(UINodeListenerPtr(new WaterHeightListener(_mapEditor, waterHeightNode)));
	hbox->add(new UINodeSetting(_frontend, tr("Waterheight"), waterHeightNode));
	new UINodeSettingsSpinner(hbox, 0, 100000, 500, _mapEditor, tr("Rising delay"), msn::WATER_RISING_DELAY, tr("Initial water rising delay in milliseconds"));
	new UINodeSettingsSpinner(hbox, 0, 100000, 500, _mapEditor, tr("Fall delay"), msn::WATER_FALLING_DELAY, tr("Water fall delay in milliseconds"));
	new UINodeSettingsSlider(hbox, 0.0f, 2.0f, 0.1f, _mapEditor, tr("Waterspeed"), msn::WATER_CHANGE, tr("The water change speed"));
	vbox->add(hbox);
	return hbox;
}

UINode* UIMapEditorOptionsWindow::createWinConditions (UINode* vbox)
{
	UINode *hbox = createSection(vbox, tr("Win conditions"));
	new UINodeSettingsSpinner(hbox, 10, 500, 1, _mapEditor, tr("Points"), msn::POINTS, tr("The points you get for finishing the map"));
	new UINodeSettingsSpinner(hbox, 1, 100, 1, _mapEditor, tr("Packages"), msn::PACKAGE_TRANSFER_COUNT, tr("The amount of packages to deliver"));
	new UINodeSettingsSpinner(hbox, 1, 100, 1, _mapEditor, tr("Time"), msn::REFERENCETIME, tr("Reference time in seconds"));
	new UINodeSettingsCheckbox(hbox, _mapEditor, tr("SideScroll"), msn::SIDEBORDERFAIL, tr("A player will lose the map if he touches the side borders"));
	vbox->add(hbox);
	return hbox;
}

UINode* UIMapEditorOptionsWindow::createGeneralOptions (UINode* vbox)
{
	const float checkBoxWidth = 36.0f / _frontend->getWidth();
	const float checkBoxHeight = 36.0f / _frontend->getHeight();

	UINode *hbox = createSection(vbox, tr("General"));
	UINodeTextInput *nameNode = new UINodeTextInput(_frontend, "", 14);
	nameNode->setBackgroundColor(colorWhite);
	nameNode->addListener(UINodeListenerPtr(new NameListener(_mapEditor, nameNode, false)));
	hbox->add(new UINodeSetting(_frontend, tr("Name"), nameNode));
	new UINodeSettingsSlider(hbox, 0.0f, 20.0f, 0.1f, _mapEditor, tr("Gravity"), msn::GRAVITY);
	new UINodeSettingsSlider(hbox, 0.0f, 6.0f, 0.1f, _mapEditor, tr("Wind"), msn::WIND);
	new UINodeSettingsSpinner(hbox, 0, 200000, 500, _mapEditor, tr("Npc delay"), msn::NPC_INITIAL_SPAWN_TIME, tr("Initial npc spawn delay for the pterodactyls and the fish"));
	new UINodeSettingsCheckbox(hbox, _mapEditor, tr("Pterodactyls"), msn::FLYING_NPC, tr("Activate the pterodactyls spawn"));
	new UINodeSettingsCheckbox(hbox, _mapEditor, tr("Fish"), msn::FISH_NPC, tr("Activate the fish spawn"));
	new UINodeSettingsCheckbox(hbox, _mapEditor, tr("Tutorial"), msn::TUTORIAL, tr("Show help messages to the player"));
	UINodeCheckbox *gridCheckBox = new UINodeCheckbox(_frontend, "grid");
	gridCheckBox->setSize(checkBoxWidth, checkBoxHeight);
	gridCheckBox->setFont(getFont(), colorBlack);
	gridCheckBox->addListener(UINodeListenerPtr(new ToggleGridListener(_mapEditor)));
	hbox->add(new UINodeSetting(_frontend, tr("Show Grid"), gridCheckBox));
	vbox->add(hbox);
	return hbox;
}
