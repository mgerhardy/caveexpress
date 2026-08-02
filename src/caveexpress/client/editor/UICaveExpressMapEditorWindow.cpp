#include "UICaveExpressMapEditorWindow.h"
#include "imgui.h"
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
	if (ImGui::InputInt("Points", &points))
		doc.setSetting(msn::POINTS, string::toString(points));
	int refTime = string::toInt(doc.getSetting(msn::REFERENCETIME, string::toString(msdv::REFERENCETIME)));
	if (ImGui::InputInt("Reference time", &refTime))
		doc.setSetting(msn::REFERENCETIME, string::toString(refTime));
	float gravity = string::toFloat(doc.getSetting(msn::GRAVITY, string::toString(msdv::GRAVITY)));
	if (ImGui::InputFloat("Gravity", &gravity))
		doc.setSetting(msn::GRAVITY, string::toString(gravity));
	float wind = string::toFloat(doc.getSetting(msn::WIND, msd::WIND));
	if (ImGui::InputFloat("Wind", &wind))
		doc.setSetting(msn::WIND, string::toString(wind));
	float water = doc.getWaterHeight();
	if (ImGui::InputFloat("Water height", &water))
		doc.setWaterHeight(water);
	int packages = string::toInt(doc.getSetting(msn::PACKAGE_TRANSFER_COUNT, msd::PACKAGE_TRANSFER_COUNT));
	if (ImGui::InputInt("Package transfers", &packages))
		doc.setSetting(msn::PACKAGE_TRANSFER_COUNT, string::toString(packages));
	bool flying = string::toBool(doc.getSetting(msn::FLYING_NPC, msd::FLYING_NPC));
	if (ImGui::Checkbox("Flying NPC", &flying))
		doc.setSetting(msn::FLYING_NPC, flying ? "true" : "false");
	bool fish = string::toBool(doc.getSetting(msn::FISH_NPC, msd::FISH_NPC));
	if (ImGui::Checkbox("Fish NPC", &fish))
		doc.setSetting(msn::FISH_NPC, fish ? "true" : "false");

	int caveDelay = doc.getCaveDelay();
	if (ImGui::InputInt("Cave delay", &caveDelay))
		doc.setCaveDelay(caveDelay);

	ImGui::Separator();
	ImGui::TextUnformatted("Theme");
	const ThemeType* themes[] = { &ThemeTypes::ROCK, &ThemeTypes::ICE, &ThemeTypes::JUNGLE, &ThemeTypes::DESERT };
	for (const ThemeType* theme : themes) {
		const bool selected = &doc.getTheme() == theme;
		if (ImGui::RadioButton(theme->name.c_str(), selected)) {
			doc.changeMapTheme(*theme);
			rebuildPalettes();
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
	if (ImGui::Button("Auto-generate")) {
		doc.autoFill(doc.getTheme());
		fitView();
	}
}

}
