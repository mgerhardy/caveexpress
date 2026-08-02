#include "UICaveExpressMapEditorWindow.h"
#include "imgui.h"
#include "ui/UI.h"
#include "common/String.h"
#include "common/MapSettings.h"
#include "common/ThemeType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include <memory>

namespace caveexpress {

UICaveExpressMapEditorWindow::UICaveExpressMapEditorWindow (IFrontend* frontend, IMapManager& mapManager) :
		UIMapEditorWindow(frontend, std::unique_ptr<IMapEditorDocument>(new MapEditorDocument(mapManager)))
{
}

const Animation& UICaveExpressMapEditorWindow::getPlayerAnimation () const
{
	return Animations::ANIMATION_IDLE;
}

void UICaveExpressMapEditorWindow::renderCanvasOverlay (ImDrawList* drawList, float originX, float originY,
		float tileW, float tileH) const
{
	if (drawList == nullptr)
		return;
	MapEditorDocument& doc = ceDocument();
	const float waterY = doc.getMapHeight() - doc.getWaterHeight();
	const float waterPixelSurface = originY + waterY * tileH - _panY;
	const float h = _canvasMaxY - _canvasMinY;
	if (waterPixelSurface >= originY + h)
		return;
	const float waterPixelHeight = doc.getWaterHeight() * tileH;
	const float waterPixelWidth = doc.getMapWidth() * tileW;
	const float waterX = originX - _panX;
	drawList->AddLine(ImVec2(waterX, waterPixelSurface), ImVec2(waterX + waterPixelWidth, waterPixelSurface),
			IM_COL32(90, 170, 230, 255));
	drawList->AddRectFilled(ImVec2(waterX, waterPixelSurface + 1.0f),
			ImVec2(waterX + waterPixelWidth, waterPixelSurface + 1.0f + waterPixelHeight),
			IM_COL32(58, 118, 181, 150));
}

void UICaveExpressMapEditorWindow::drawPropertiesPanel () const
{
	UIMapEditorWindow::drawPropertiesPanel();
	MapEditorDocument& doc = ceDocument();

	int points = string::toInt(doc.getSetting(msn::POINTS, string::toString(msdv::POINTS)));
	if (ImGui::InputInt(tr("Points").c_str(), &points))
		doc.setSetting(msn::POINTS, string::toString(points));
	int refTime = string::toInt(doc.getSetting(msn::REFERENCETIME, string::toString(msdv::REFERENCETIME)));
	if (ImGui::InputInt(tr("Reference time in seconds").c_str(), &refTime))
		doc.setSetting(msn::REFERENCETIME, string::toString(refTime));
	float gravity = string::toFloat(doc.getSetting(msn::GRAVITY, string::toString(msdv::GRAVITY)));
	if (ImGui::InputFloat(tr("Gravity").c_str(), &gravity))
		doc.setSetting(msn::GRAVITY, string::toString(gravity));
	float wind = string::toFloat(doc.getSetting(msn::WIND, msd::WIND));
	if (ImGui::InputFloat(tr("Wind").c_str(), &wind))
		doc.setSetting(msn::WIND, string::toString(wind));
	float water = doc.getWaterHeight();
	if (ImGui::InputFloat(tr("Waterheight").c_str(), &water))
		doc.setWaterHeight(water);
	int packages = string::toInt(doc.getSetting(msn::PACKAGE_TRANSFER_COUNT, msd::PACKAGE_TRANSFER_COUNT));
	if (ImGui::InputInt(tr("The amount of packages to deliver").c_str(), &packages))
		doc.setSetting(msn::PACKAGE_TRANSFER_COUNT, string::toString(packages));
	bool flying = string::toBool(doc.getSetting(msn::FLYING_NPC, msd::FLYING_NPC));
	if (ImGui::Checkbox(tr("Activate the pterodactyls spawn").c_str(), &flying))
		doc.setSetting(msn::FLYING_NPC, flying ? "true" : "false");
	bool fish = string::toBool(doc.getSetting(msn::FISH_NPC, msd::FISH_NPC));
	if (ImGui::Checkbox(tr("Activate the fish spawn").c_str(), &fish))
		doc.setSetting(msn::FISH_NPC, fish ? "true" : "false");

	int caveDelay = doc.getCaveDelay();
	if (ImGui::InputInt(tr("Npc delay").c_str(), &caveDelay))
		doc.setCaveDelay(caveDelay);

	ImGui::Separator();
	ImGui::TextUnformatted(tr("Theme").c_str());
	const struct {
		const ThemeType* theme;
		std::string label;
	} themes[] = {
		{ &ThemeTypes::ROCK, tr("Rock") },
		{ &ThemeTypes::ICE, tr("Ice") },
		{ &ThemeTypes::JUNGLE, tr("Jungle") },
		{ &ThemeTypes::DESERT, tr("Desert") },
	};
	for (const auto& entry : themes) {
		const bool selected = &doc.getTheme() == entry.theme;
		if (ImGui::RadioButton(entry.label.c_str(), selected)) {
			doc.changeMapTheme(*entry.theme);
			rebuildPalettes();
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
	if (ImGui::Button(tr("Auto").c_str())) {
		doc.autoFill(doc.getTheme());
		fitView();
	}
}

}
