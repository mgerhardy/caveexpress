#include "UICaveExpressMapEditorWindow.h"
#include "imgui.h"
#include "ui/UI.h"
#include "common/String.h"
#include "common/MapSettings.h"
#include "common/ThemeType.h"
#include "common/Math.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include <memory>
#include <cstdio>
#include <algorithm>

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
	if (waterPixelSurface < originY + h) {
		const float waterPixelHeight = doc.getWaterHeight() * tileH;
		const float waterPixelWidth = doc.getMapWidth() * tileW;
		const float waterX = originX - _panX;
		drawList->AddLine(ImVec2(waterX, waterPixelSurface), ImVec2(waterX + waterPixelWidth, waterPixelSurface),
				IM_COL32(90, 170, 230, 255));
		drawList->AddRectFilled(ImVec2(waterX, waterPixelSurface + 1.0f),
				ImVec2(waterX + waterPixelWidth, waterPixelSurface + 1.0f + waterPixelHeight),
				IM_COL32(58, 118, 181, 150));
	}

	const MapEditorTileItem* focus = doc.getHighlightItem();
	if (focus == nullptr || focus->def == nullptr || focus->linkId.empty())
		return;
	if (!SpriteTypes::isGate(focus->def->type) && !SpriteTypes::isPressurePlate(focus->def->type))
		return;
	MapEditorTileItem* partner = doc.findLinkedPartner(*focus);
	if (partner == nullptr)
		return;
	const float ax = originX + (focus->gridX + 0.5f) * tileW - _panX;
	const float ay = originY + (focus->gridY + 0.5f) * tileH - _panY;
	const float bx = originX + (partner->gridX + 0.5f) * tileW - _panX;
	const float by = originY + (partner->gridY + 0.5f) * tileH - _panY;
	drawList->AddLine(ImVec2(ax, ay), ImVec2(bx, by), IM_COL32(255, 200, 40, 220), 2.0f);
	drawList->AddCircleFilled(ImVec2(ax, ay), 4.0f, IM_COL32(255, 200, 40, 255));
	drawList->AddCircleFilled(ImVec2(bx, by), 4.0f, IM_COL32(255, 200, 40, 255));
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

	MapEditorTileItem* sel = doc.getHighlightItem();
	if (sel != nullptr && sel->def != nullptr
			&& (SpriteTypes::isGate(sel->def->type) || SpriteTypes::isPressurePlate(sel->def->type))) {
		ImGui::Separator();
		ImGui::TextUnformatted(tr("Trigger link").c_str());
		char linkBuf[64];
		std::snprintf(linkBuf, sizeof(linkBuf), "%s", sel->linkId.c_str());
		if (ImGui::InputText(tr("Link id").c_str(), linkBuf, sizeof(linkBuf)))
			sel->linkId = linkBuf;
		if (SpriteTypes::isPressurePlate(sel->def->type)) {
			float weight = sel->requiredWeight;
			if (ImGui::InputFloat(tr("Required weight").c_str(), &weight))
				sel->requiredWeight = std::max(0.0f, weight);
			int hold = sel->delay;
			if (ImGui::InputInt(tr("Hold ms").c_str(), &hold))
				sel->delay = std::max(0, hold);
			if (doc.isPickingGateTarget()) {
				ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", tr("Click a gate to link").c_str());
				if (ImGui::Button(tr("Cancel pick").c_str()))
					doc.cancelPickGateTarget();
			} else if (ImGui::Button(tr("Pick gate target").c_str())) {
				doc.beginPickGateTarget();
			}
		} else if (SpriteTypes::isGate(sel->def->type)) {
			float openAmount = sel->openAmount;
			if (ImGui::InputFloat(tr("Open amount").c_str(), &openAmount))
				sel->openAmount = clamp(openAmount, 0.0f, 1.0f);
		}
	}

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
	int seed = string::toInt(doc.getSetting("seed", "0"));
	if (ImGui::InputInt(tr("Seed").c_str(), &seed))
		doc.setSetting("seed", string::toString(std::max(0, seed)));
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Map generator seed. 0 picks a new seed on Auto.").c_str());
	ImGui::SameLine();
	if (ImGui::Button(tr("Auto").c_str())) {
		doc.autoFill(doc.getTheme());
		fitView();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Generate a random map with the current seed.").c_str());
}

}
