#include "IUIMapEditorOptionsWindow.h"
#include "ui/nodes/IUINodeEntitySelector.h"
#include "ui/nodes/IUINodeSpriteSelector.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/IUINodeMapEditor.h"

#include "ui/nodes/UINodeBackButton.h"
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
#include "ui/windows/IUIMapWindow.h"
#include "ui/windows/IUIMapEditorWindow.h"
#include "ui/windows/UISettingsWindow.h"
#include "ui/windows/mapeditor/QuitEditorListener.h"
#include "ui/nodes/UINodeMapEditorSelectedItem.h"
#include "ui/nodes/UINodeMapStringSelector.h"

#include "mapeditor/IntSettingsListener.h"
#include "mapeditor/FloatSettingsListener.h"
#include "mapeditor/SaveListener.h"
#include "mapeditor/LoadListener.h"
#include "mapeditor/LoadListListener.h"
#include "mapeditor/NameListener.h"
#include "mapeditor/NewListener.h"
#include "mapeditor/UndoListener.h"
#include "mapeditor/RedoListener.h"
#include "mapeditor/SpriteSelectionListener.h"
#include "mapeditor/ToggleGridListener.h"
#include "mapeditor/EntitySelectionListener.h"
#include "mapeditor/BooleanSettingListener.h"
#include "mapeditor/LayerListener.h"

UINodeSettingsCheckbox::UINodeSettingsCheckbox (UINode* parent, IUINodeMapEditor* mapEditor, const std::string& label, const std::string& setting, const std::string& tooltip) :
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

UINodeSettingsSpinner::UINodeSettingsSpinner (UINode* parent, int min, int max, int stepWidth, IUINodeMapEditor* mapEditor,
		const std::string& label, const std::string& setting, const std::string& tooltip) :
		UINodeSpinner(parent->getFrontend(), min, max, stepWidth) {
	UINodeListener* listener = new IntSettingsListener(mapEditor, this, setting);
	setId(label);
	setBackgroundColor(colorWhite);
	setSize(0.05f, 0.025f);
	setTooltip(tooltip);
	addListener(UINodeListenerPtr(listener));
	parent->add(new UINodeSetting(_frontend, label, this));
}

UINodeSettingsSlider::UINodeSettingsSlider (UINode* parent, float min, float max, float stepWidth, IUINodeMapEditor* mapEditor, const std::string& label, const std::string& setting, const std::string& tooltip) :
		UINodeSlider(parent->getFrontend(), min, max, stepWidth) {
	UINodeListener* listener = new FloatSettingsListener(mapEditor, this, setting);
	setId(label);
	setSize(0.1f, 0.025f);
	setTooltip(tooltip);
	addListener(UINodeListenerPtr(listener));
	parent->add(new UINodeSetting(_frontend, label, this));
}

IUIMapEditorOptionsWindow::IUIMapEditorOptionsWindow (IFrontend *frontend, IUINodeMapEditor* mapEditor) :
		UIWindow(UI_WINDOW_MAPEDITOR_OPTIONS, frontend, WINDOW_FLAG_MODAL), _mapEditor(mapEditor), _vbox(nullptr)
{
}

void IUIMapEditorOptionsWindow::init()
{
	UINode *background = new UINode(_frontend, "background");
	background->setBackgroundColor(colorDark);
	background->setSize(1.0f, 1.0f);

	_vbox = new UINode(_frontend, "vbox");
	_vbox->setLayout(new UIVBoxLayout(0.02f, true, NODE_ALIGN_CENTER));

	createGeneralOptions(_vbox);
	createWinConditions(_vbox);

	background->add(_vbox);
	add(background);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(_frontend, background));
}

IUIMapEditorOptionsWindow::~IUIMapEditorOptionsWindow ()
{
}

UINode* IUIMapEditorOptionsWindow::createSection (UINode* parent, const std::string& title)
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

UINode* IUIMapEditorOptionsWindow::createWinConditions (UINode* vbox)
{
	UINode *hbox = createSection(vbox, tr("Win conditions"));
	fillWinConditions(hbox);
	vbox->add(hbox);
	return hbox;
}

void IUIMapEditorOptionsWindow::fillWinConditions (UINode* hbox)
{
	new UINodeSettingsSpinner(hbox, 1, 100, 1, _mapEditor, tr("Time"), msn::REFERENCETIME, tr("Reference time in seconds"));
}

UINode* IUIMapEditorOptionsWindow::createGeneralOptions (UINode* vbox)
{
	UINode *hbox = createSection(vbox, tr("General"));
	fillGeneralOptions(hbox);
	vbox->add(hbox);
	return hbox;
}

void IUIMapEditorOptionsWindow::fillGeneralOptions (UINode* hbox)
{
	const float checkBoxWidth = 36.0f / _frontend->getWidth();
	const float checkBoxHeight = 36.0f / _frontend->getHeight();

	UINodeTextInput *nameNode = new UINodeTextInput(_frontend, "", 14);
	nameNode->setBackgroundColor(colorWhite);
	nameNode->addListener(UINodeListenerPtr(new NameListener(_mapEditor, nameNode, false)));
	hbox->add(new UINodeSetting(_frontend, tr("Name"), nameNode));
	new UINodeSettingsCheckbox(hbox, _mapEditor, tr("Tutorial"), msn::TUTORIAL, tr("Show help messages to the player"));
	UINodeCheckbox *gridCheckBox = new UINodeCheckbox(_frontend, "grid");
	gridCheckBox->setSize(checkBoxWidth, checkBoxHeight);
	gridCheckBox->setFont(getFont(), colorBlack);
	gridCheckBox->addListener(UINodeListenerPtr(new ToggleGridListener(_mapEditor)));
	hbox->add(new UINodeSetting(_frontend, tr("Show Grid"), gridCheckBox));
}
