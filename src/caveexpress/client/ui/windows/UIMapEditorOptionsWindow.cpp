#include "UIMapEditorOptionsWindow.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeCheckbox.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSlider.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeSpinner.h"
#include "ui/nodes/UINodeTextInput.h"
#include "ui/nodes/UINodeButtonText.h"
#include "ui/nodes/UINodeMapEditorSelectedItem.h"
#include "ui/nodes/UINodeMapStringSelector.h"

#include "ui/windows/IUIMapEditorWindow.h"
#include "ui/windows/UISettingsWindow.h"

#include "caveexpress/client/ui/nodes/UINodeMapEditor.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"
#include "caveexpress/client/ui/nodes/UINodeSpriteSelector.h"
#include "caveexpress/client/ui/nodes/UINodeEntitySelector.h"
#include "caveexpress/shared/constants/Commands.h"
#include "mapeditor/WaterHeightListener.h"

namespace caveexpress {

namespace {
const float height = 0.025f;
const float sliderWidth = 0.1f;
}

UIMapEditorOptionsWindow::UIMapEditorOptionsWindow (IFrontend *frontend, UINodeMapEditor* mapEditor) :
		IUIMapEditorOptionsWindow(frontend, mapEditor)
{
	init();
}

UIMapEditorOptionsWindow::~UIMapEditorOptionsWindow ()
{
}

UINode* UIMapEditorOptionsWindow::createGeneralOptions (UINode* vbox)
{
	UINode* node = IUIMapEditorOptionsWindow::createGeneralOptions(vbox);
	createWaterOptions(vbox);
	return node;
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

void UIMapEditorOptionsWindow::fillWinConditions (UINode* hbox)
{
	IUIMapEditorOptionsWindow::fillWinConditions(hbox);
	new UINodeSettingsSpinner(hbox, 10, 500, 1, _mapEditor, tr("Points"), msn::POINTS, tr("The points you get for finishing the map"));
	new UINodeSettingsSpinner(hbox, 1, 100, 1, _mapEditor, tr("Npcs"), msn::NPC_TRANSFER_COUNT, tr("The amount of npcs to deliver"));
	new UINodeSettingsSpinner(hbox, 1, 100, 1, _mapEditor, tr("Packages"), msn::PACKAGE_TRANSFER_COUNT, tr("The amount of packages to deliver"));
	new UINodeSettingsCheckbox(hbox, _mapEditor, tr("SideScroll"), msn::SIDEBORDERFAIL, tr("A player will lose the map if he touches the side borders"));
}

void UIMapEditorOptionsWindow::fillGeneralOptions (UINode* hbox)
{
	IUIMapEditorOptionsWindow::fillGeneralOptions(hbox);
	new UINodeSettingsSlider(hbox, 0.0f, 20.0f, 0.1f, _mapEditor, tr("Gravity"), msn::GRAVITY);
	new UINodeSettingsSlider(hbox, 0.0f, 6.0f, 0.1f, _mapEditor, tr("Wind"), msn::WIND);
	new UINodeSettingsSpinner(hbox, 0, 100, 1, _mapEditor, tr("Npcs"), msn::NPCS, tr("This is the max amount of npcs that spawn, if you let more than these die, you lose the game"));
	new UINodeSettingsSpinner(hbox, 0, 200000, 500, _mapEditor, tr("Npc delay"), msn::NPC_INITIAL_SPAWN_TIME, tr("Initial npc spawn delay for the pterodactyls and the fish"));
	new UINodeSettingsCheckbox(hbox, _mapEditor, tr("Pterodactyls"), msn::FLYING_NPC, tr("Activate the pterodactyls spawn"));
	new UINodeSettingsCheckbox(hbox, _mapEditor, tr("Fish"), msn::FISH_NPC, tr("Activate the fish spawn"));
}

}
