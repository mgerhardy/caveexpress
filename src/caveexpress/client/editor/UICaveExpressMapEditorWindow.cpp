#include "UICaveExpressMapEditorWindow.h"
#include "imgui.h"
#include "ui/UI.h"
#include "common/String.h"
#include "common/Log.h"
#include "common/MapSettings.h"
#include "common/ThemeType.h"
#include "common/Math.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/constants/EmitterSettings.h"
#include "common/KeyValueParser.h"
#include "common/IMap.h"
#include <memory>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <vector>

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
				IM_COL32(90, 170, 230, 255), 2.0f);
		drawList->AddText(ImVec2(waterX + 6.0f, waterPixelSurface - 16.0f), IM_COL32(140, 200, 240, 220),
				tr("Alt+drag water").c_str());
		drawList->AddRectFilled(ImVec2(waterX, waterPixelSurface + 1.0f),
				ImVec2(waterX + waterPixelWidth, waterPixelSurface + 1.0f + waterPixelHeight),
				IM_COL32(58, 118, 181, 150));
	}

	const float wind = string::toFloat(doc.getSetting(msn::WIND, msd::WIND));
	if (std::fabs(wind) > 0.0001f) {
		const float dir = wind > 0.0f ? 1.0f : -1.0f;
		const float y = originY + 10.0f - _panY;
		const ImU32 col = IM_COL32(180, 210, 255, 200);
		for (int i = 1; i < doc.getMapWidth(); i += 3) {
			const float x = originX + static_cast<float>(i) * tileW - _panX;
			drawList->AddLine(ImVec2(x, y), ImVec2(x + dir * 18.0f, y), col, 2.0f);
			drawList->AddLine(ImVec2(x + dir * 18.0f, y), ImVec2(x + dir * 12.0f, y - 5.0f), col, 2.0f);
			drawList->AddLine(ImVec2(x + dir * 18.0f, y), ImVec2(x + dir * 12.0f, y + 5.0f), col, 2.0f);
		}
	}

	auto drawLink = [&] (const MapEditorTileItem& a, const MapEditorTileItem& b, ImU32 col) {
		const float ax = originX + (a.gridX + 0.5f) * tileW - _panX;
		const float ay = originY + (a.gridY + 0.5f) * tileH - _panY;
		const float bx = originX + (b.gridX + 0.5f) * tileW - _panX;
		const float by = originY + (b.gridY + 0.5f) * tileH - _panY;
		drawList->AddLine(ImVec2(ax, ay), ImVec2(bx, by), col, 2.0f);
		drawList->AddCircleFilled(ImVec2(ax, ay), 4.0f, col);
		drawList->AddCircleFilled(ImVec2(bx, by), 4.0f, col);
	};

	if (_showAllTriggerLinks) {
		for (const MapEditorTileItem& item : doc.getTiles()) {
			if (!item.def || item.linkId.empty() || !SpriteTypes::isPressurePlate(item.def->type))
				continue;
			const MapEditorTileItem* partner = doc.findLinkedPartner(item);
			if (partner != nullptr)
				drawLink(item, *partner, IM_COL32(255, 200, 40, 140));
		}
	}

	const MapEditorTileItem* focus = doc.getHighlightItem();
	if (focus == nullptr || focus->def == nullptr || focus->linkId.empty())
		return;
	if (!SpriteTypes::isGate(focus->def->type) && !SpriteTypes::isPressurePlate(focus->def->type))
		return;
	const MapEditorTileItem* partner = doc.findLinkedPartner(*focus);
	if (partner == nullptr)
		return;
	drawLink(*focus, *partner, IM_COL32(255, 200, 40, 255));
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
	int npcs = string::toInt(doc.getSetting(msn::NPC_TRANSFER_COUNT, msd::NPC_TRANSFER_COUNT));
	if (ImGui::InputInt(tr("Friendly NPCs to deliver").c_str(), &npcs))
		doc.setSetting(msn::NPC_TRANSFER_COUNT, string::toString(std::max(0, npcs)));
	int npcCap = string::toInt(doc.getSetting(msn::NPCS, msd::NPCS));
	if (ImGui::InputInt(tr("Friendly NPC spawn cap").c_str(), &npcCap))
		doc.setSetting(msn::NPCS, string::toString(std::max(0, npcCap)));
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("npcs: how many friendly villagers can be alive from caves").c_str());
	bool flying = string::toBool(doc.getSetting(msn::FLYING_NPC, msd::FLYING_NPC));
	if (ImGui::Checkbox(tr("Activate the pterodactyls spawn").c_str(), &flying))
		doc.setSetting(msn::FLYING_NPC, flying ? "true" : "false");
	bool fish = string::toBool(doc.getSetting(msn::FISH_NPC, msd::FISH_NPC));
	if (ImGui::Checkbox(tr("Activate the fish spawn").c_str(), &fish))
		doc.setSetting(msn::FISH_NPC, fish ? "true" : "false");
	int spawnTime = string::toInt(doc.getSetting(msn::NPC_INITIAL_SPAWN_TIME, "0"));
	if (ImGui::InputInt(tr("First flying/fish spawn delay").c_str(), &spawnTime))
		doc.setSetting(msn::NPC_INITIAL_SPAWN_TIME, string::toString(std::max(0, spawnTime)));
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("initialspawntime in ms. 0 lets the engine pick a random delay.").c_str());
	int geyserDelay = string::toInt(doc.getSetting(msn::GEYSER_INITIAL_DELAY_TIME, "3000"));
	if (ImGui::InputInt(tr("Geyser initial delay").c_str(), &geyserDelay))
		doc.setSetting(msn::GEYSER_INITIAL_DELAY_TIME, string::toString(std::max(0, geyserDelay)));
	bool sideFail = string::toBool(doc.getSetting(msn::SIDEBORDERFAIL, msd::SIDEBORDERFAIL));
	if (ImGui::Checkbox(tr("Fail on side border").c_str(), &sideFail))
		doc.setSetting(msn::SIDEBORDERFAIL, sideFail ? "true" : "false");
	bool tutorial = string::toBool(doc.getSetting(msn::TUTORIAL, msd::TUTORIAL));
	if (ImGui::Checkbox(tr("Tutorial").c_str(), &tutorial))
		doc.setSetting(msn::TUTORIAL, tutorial ? "true" : "false");
	bool cutscene = string::toBool(doc.getSetting(msn::CUTSCENE, msd::CUTSCENE));
	if (ImGui::Checkbox(tr("Cutscene").c_str(), &cutscene))
		doc.setSetting(msn::CUTSCENE, cutscene ? "true" : "false");
	const char* introIds[] = {
		"", "intropackage", "introtime", "introtree", "introgeyser", "introflying",
		"introattack", "introfindyourway", "introdiving"
	};
	const char* introLabels[] = {
		"(none)", "intropackage", "introtime", "introtree", "introgeyser", "introflying",
		"introattack", "introfindyourway", "introdiving"
	};
	std::string intro = doc.getSetting(msn::INTROWINDOW, msd::INTROWINDOW);
	int introIdx = 0;
	for (int i = 0; i < 9; ++i) {
		if (intro == introIds[i])
			introIdx = i;
	}
	if (ImGui::Combo(tr("Intro window").c_str(), &introIdx, introLabels, 9))
		doc.setSetting(msn::INTROWINDOW, introIds[introIdx]);

	float waterChange = string::toFloat(doc.getSetting(msn::WATER_CHANGE, msd::WATER_CHANGE));
	if (ImGui::InputFloat(tr("Water change speed").c_str(), &waterChange))
		doc.setSetting(msn::WATER_CHANGE, string::toString(waterChange));
	int waterRise = string::toInt(doc.getSetting(msn::WATER_RISING_DELAY, msd::WATER_RISING_DELAY));
	if (ImGui::InputInt(tr("Water rising delay").c_str(), &waterRise))
		doc.setSetting(msn::WATER_RISING_DELAY, string::toString(std::max(0, waterRise)));
	int waterFall = string::toInt(doc.getSetting(msn::WATER_FALLING_DELAY, msd::WATER_FALLING_DELAY));
	if (ImGui::InputInt(tr("Water falling delay").c_str(), &waterFall))
		doc.setSetting(msn::WATER_FALLING_DELAY, string::toString(std::max(0, waterFall)));

	int caveDelay = doc.getCaveDelay();
	if (ImGui::InputInt(tr("Npc delay").c_str(), &caveDelay))
		doc.setCaveDelay(caveDelay);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Default spawn delay for newly placed caves").c_str());

	const EntityType* caveTypes[] = {
		&EntityType::NONE, &EntityTypes::NPC_FRIENDLY_MAN, &EntityTypes::NPC_FRIENDLY_WOMAN,
		&EntityTypes::NPC_FRIENDLY_GRANDPA
	};
	const char* caveTypeLabels[] = { "none", "npc-man", "npc-woman", "npc-grandpa" };
	int brushType = 0;
	for (int i = 0; i < 4; ++i) {
		if (&doc.getCaveNpcType() == caveTypes[i])
			brushType = i;
	}
	if (ImGui::Combo(tr("Cave NPC (brush)").c_str(), &brushType, caveTypeLabels, 4))
		doc.setCaveNpcType(*caveTypes[brushType]);

	MapEditorTileItem* sel = doc.getHighlightItem();
	if (sel != nullptr && sel->def != nullptr
			&& (SpriteTypes::isGate(sel->def->type) || SpriteTypes::isPressurePlate(sel->def->type))) {
		ImGui::Separator();
		ImGui::TextUnformatted(tr("Trigger link").c_str());
		char linkBuf[64];
		std::snprintf(linkBuf, sizeof(linkBuf), "%s", sel->linkId.c_str());
		if (ImGui::InputText(tr("Link id").c_str(), linkBuf, sizeof(linkBuf)))
			sel->linkId = linkBuf;
		ImGui::SameLine();
		if (ImGui::Button(tr("Copy").c_str()))
			ImGui::SetClipboardText(sel->linkId.c_str());
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

	if (sel != nullptr && sel->def != nullptr && SpriteTypes::isCave(sel->def->type)) {
		ImGui::Separator();
		ImGui::TextUnformatted(tr("Cave").c_str());
		int caveSel = 0;
		for (int i = 0; i < 4; ++i) {
			if (sel->entityType == caveTypes[i])
				caveSel = i;
		}
		if (ImGui::Combo(tr("Spawn NPC").c_str(), &caveSel, caveTypeLabels, 4))
			sel->entityType = caveTypes[caveSel];
		int delay = sel->delay;
		if (ImGui::InputInt(tr("Spawn delay ms").c_str(), &delay))
			sel->delay = std::max(0, delay);
	}

	if (sel != nullptr && sel->def != nullptr && sel->def->rotateable > 0) {
		int angle = sel->angle;
		if (ImGui::InputInt(tr("Tile angle").c_str(), &angle))
			sel->angle = static_cast<EntityAngle>(((angle % 360) + 360) % 360);
		if (ImGui::Button(tr("Rotate selected").c_str()))
			doc.rotateSelectionOrBrush();
	}

	if (sel != nullptr && sel->entityType != nullptr) {
		ImGui::Separator();
		ImGui::TextUnformatted(tr("Emitter").c_str());
		float ex = sel->gridX;
		float ey = sel->gridY;
		if (ImGui::InputFloat(tr("X").c_str(), &ex, 0.1f, 1.0f, "%.2f"))
			sel->gridX = ex;
		if (ImGui::InputFloat(tr("Y").c_str(), &ey, 0.1f, 1.0f, "%.2f"))
			sel->gridY = ey;
		KeyValueParser kv(sel->settings);
		if (EntityTypes::hasDirection(*sel->entityType)) {
			bool right = kv.getBool(EMITTER_RIGHT, true);
			if (ImGui::Checkbox(tr("Faces right").c_str(), &right)) {
				if (right)
					kv.remove(EMITTER_RIGHT);
				else
					kv.set(EMITTER_RIGHT, false);
				sel->settings = kv.str();
				doc.setActiveEntityRight(right);
			}
		}
		if (EntityTypes::isNpcBlowing(*sel->entityType)) {
			float strength = kv.getFloat(EMITTER_STRENGTH, 10.0f);
			float size = kv.getFloat(EMITTER_WIND_MOD_SIZE, 2.0f);
			if (ImGui::InputFloat(tr("Blow strength").c_str(), &strength, 0.5f, 1.0f, "%.1f")) {
				kv.set(EMITTER_STRENGTH, std::max(0.0f, strength));
				sel->settings = kv.str();
			}
			if (ImGui::InputFloat(tr("Wind size").c_str(), &size, 0.5f, 1.0f, "%.1f")) {
				kv.set(EMITTER_WIND_MOD_SIZE, std::max(0.0f, size));
				sel->settings = kv.str();
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", tr("Wind column height in tiles (emitter setting size=)").c_str());
		}
	}

	ImGui::Separator();
	ImGui::Text("%s: %i", tr("Start positions").c_str(), static_cast<int>(doc.getStartPositions().size()));
	const IMap::StartPositions& starts = doc.getStartPositions();
	for (size_t i = 0; i < starts.size(); ++i) {
		ImGui::PushID(static_cast<int>(i));
		float sx = string::toFloat(starts[i]._x);
		float sy = string::toFloat(starts[i]._y);
		ImGui::SetNextItemWidth(70.0f);
		if (ImGui::InputFloat("##sx", &sx, 0.0f, 0.0f, "%.2f"))
			doc.setStartPositionAt(i, sx, sy);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70.0f);
		if (ImGui::InputFloat("##sy", &sy, 0.0f, 0.0f, "%.2f"))
			doc.setStartPositionAt(i, sx, sy);
		ImGui::SameLine();
		if (ImGui::Button(tr("Play").c_str())) {
			doc.setFileName(_fileNameBuf);
			doc.setMapName(_mapTitleBuf);
			doc.saveAndPlayFrom(sx, sy);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", tr("Save and start at this pad (god mode)").c_str());
		ImGui::SameLine();
		if (ImGui::Button(tr("Del").c_str()))
			doc.removeStartPosition(i);
		ImGui::PopID();
	}

	ImGui::Checkbox(tr("Show all trigger links").c_str(), &_showAllTriggerLinks);

	const std::vector<UnpairedTrigger> unpaired = doc.listUnpairedTriggers();
	if (!unpaired.empty()) {
		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "%s", tr("Unpaired plates / gates").c_str());
		for (size_t i = 0; i < unpaired.size(); ++i) {
			const UnpairedTrigger& t = unpaired[i];
			ImGui::PushID(static_cast<int>(i + 1000));
			ImGui::BulletText("%s @ %.0f,%.0f %s", t.kind.c_str(), t.x, t.y, t.linkId.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton(tr("Go").c_str())) {
				doc.focusCell(t.x, t.y);
				_panX = t.x * tileWidth() - (_canvasMaxX - _canvasMinX) * 0.5f;
				_panY = t.y * tileHeight() - (_canvasMaxY - _canvasMinY) * 0.5f;
			}
			ImGui::PopID();
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
	static const ThemeType* pendingTheme = nullptr;
	for (const auto& entry : themes) {
		const bool selected = &doc.getTheme() == entry.theme;
		if (ImGui::RadioButton(entry.label.c_str(), selected)) {
			int wouldReplace = 0;
			int leftover = 0;
			doc.previewThemeRemap(*entry.theme, wouldReplace, leftover);
			if (leftover > 0) {
				pendingTheme = entry.theme;
				ImGui::OpenPopup("###themeremap");
			} else {
				doc.changeMapTheme(*entry.theme);
				rebuildPalettes();
			}
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
	if (ImGui::BeginPopupModal((tr("Theme remap") + "###themeremap").c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		int wouldReplace = 0;
		int leftover = 0;
		if (pendingTheme)
			doc.previewThemeRemap(*pendingTheme, wouldReplace, leftover);
		ImGui::Text("%s: %i", tr("Tiles that will be remapped").c_str(), wouldReplace);
		ImGui::Text("%s: %i", tr("Themed tiles left unchanged").c_str(), leftover);
		if (ImGui::Button(tr("Remap").c_str()) && pendingTheme) {
			doc.changeMapTheme(*pendingTheme);
			rebuildPalettes();
			pendingTheme = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(tr("Cancel").c_str())) {
			pendingTheme = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
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
	ImGui::SameLine();
	if (ImGui::Button(tr("Make playable").c_str()))
		doc.makePlayable();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Add a cave, shredder, package, and start if they are missing.").c_str());

	ImGui::Separator();
	if (ImGui::Button(tr("Check layout").c_str())) {
		_layoutMetrics = doc.evaluateLayout();
		_layoutChecked = true;
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Run MapValidator: reachability, covered caves, flyable cells").c_str());
	if (_layoutChecked) {
		if (_layoutMetrics.valid)
			ImGui::TextColored(ImVec4(0.4f, 0.85f, 0.4f, 1.0f), "%s", tr("Layout ok").c_str());
		else
			ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.3f, 1.0f), "%s: %s", tr("Layout").c_str(),
					_layoutMetrics.failureReason.c_str());
		ImGui::Text("%s: %i / %i", tr("Caves reachable").c_str(), _layoutMetrics.cavesReachable, _layoutMetrics.caveCount);
		ImGui::Text("%s: %i / %i", tr("Targets reachable").c_str(), _layoutMetrics.packageTargetsReachable,
				_layoutMetrics.packageTargetCount);
		ImGui::Text("%s: %i / %i", tr("Flyable reachable").c_str(), _layoutMetrics.flyableReachable,
				_layoutMetrics.flyableCells);
		if (_layoutMetrics.cavesCoveredBySolid > 0)
			ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.3f, 1.0f), "%s: %i", tr("Caves covered").c_str(),
					_layoutMetrics.cavesCoveredBySolid);
	}

	ImGui::Separator();
	ImGui::TextUnformatted(tr("Campaign").c_str());
	const std::vector<std::string> inCampaigns = doc.campaignsContainingMap();
	if (!inCampaigns.empty()) {
		ImGui::TextUnformatted(tr("In campaigns").c_str());
		for (const std::string& c : inCampaigns) {
			ImGui::PushID(c.c_str());
			ImGui::BulletText("%s", c.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton(tr("Remove").c_str())) {
				if (!doc.removeFromCampaign(c))
					Log::error(LOG_UI, "Failed to remove map from campaign");
			}
			ImGui::PopID();
		}
	}
	const std::vector<std::string> campaigns = doc.listCampaignFiles();
	static int campaignIdx = 0;
	if (!campaigns.empty()) {
		campaignIdx = std::max(0, std::min(campaignIdx, static_cast<int>(campaigns.size()) - 1));
		const char* preview = campaigns[static_cast<size_t>(campaignIdx)].c_str();
		if (ImGui::BeginCombo(tr("Add to campaign").c_str(), preview)) {
			for (int i = 0; i < static_cast<int>(campaigns.size()); ++i) {
				if (ImGui::Selectable(campaigns[static_cast<size_t>(i)].c_str(), i == campaignIdx))
					campaignIdx = i;
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button(tr("Add").c_str())) {
			if (!doc.addToCampaign(campaigns[static_cast<size_t>(campaignIdx)]))
				Log::error(LOG_UI, "Failed to add map to campaign");
		}
		if (ImGui::CollapsingHeader(tr("Maps in campaign").c_str())) {
			const std::vector<std::string> maps = doc.listMapsInCampaign(campaigns[static_cast<size_t>(campaignIdx)]);
			for (const std::string& m : maps)
				ImGui::BulletText("%s", m.c_str());
		}
	}
	static char newCampaignFile[64] = {};
	static char newCampaignId[64] = {};
	static char newCampaignText[64] = {};
	ImGui::InputText(tr("New campaign file").c_str(), newCampaignFile, sizeof(newCampaignFile));
	ImGui::InputText(tr("Campaign id").c_str(), newCampaignId, sizeof(newCampaignId));
	ImGui::InputText(tr("Campaign text").c_str(), newCampaignText, sizeof(newCampaignText));
	if (ImGui::Button(tr("Create campaign").c_str())) {
		if (!doc.createCampaign(newCampaignFile, newCampaignId, newCampaignText))
			Log::error(LOG_UI, "Failed to create campaign (exists, empty id, or quote in text)");
	}

	if (ImGui::CollapsingHeader(tr("Extra settings").c_str())) {
		static const char* known[] = {
			"width", "height", "points", "tutorial", "npcs", "introwindow", "referencetime",
			"sideborderfail", "packagetransfercount", "npctransfercount", "initialspawntime",
			"geyserinitialdelay", "flyingnpc", "fishnpc", "wind", "gravity", "waterheight",
			"waterchangespeed", "waterrisingdelay", "waterfallingdelay", "theme", "cutscene", "seed"
		};
		ImGui::TextWrapped("%s", tr("Settings without a dedicated control. Saved with the map.").c_str());
		const IMap::SettingsMap settings = doc.getSettings();
		for (const auto& kv : settings) {
			bool hide = false;
			for (const char* k : known) {
				if (kv.first == k) {
					hide = true;
					break;
				}
			}
			if (hide)
				continue;
			ImGui::PushID(kv.first.c_str());
			char val[128];
			std::snprintf(val, sizeof(val), "%s", kv.second.c_str());
			if (ImGui::InputText(kv.first.c_str(), val, sizeof(val)))
				doc.setSetting(kv.first, val);
			ImGui::SameLine();
			if (ImGui::SmallButton(tr("Del").c_str()))
				doc.removeSetting(kv.first);
			ImGui::PopID();
		}
		static char extraKey[64] = {};
		static char extraVal[128] = {};
		ImGui::InputText(tr("Key").c_str(), extraKey, sizeof(extraKey));
		ImGui::InputText(tr("Value").c_str(), extraVal, sizeof(extraVal));
		if (ImGui::Button(tr("Add setting").c_str()) && extraKey[0] != '\0') {
			doc.setSetting(extraKey, extraVal);
			extraKey[0] = '\0';
			extraVal[0] = '\0';
		}
	}
}

void UICaveExpressMapEditorWindow::drawScriptExtras () const
{
	MapEditorDocument& doc = ceDocument();
	if (ImGui::Button(tr("Insert cutscene lock").c_str())) {
		const char* snippet =
				"function onMapLoaded()\n"
				"\tlocal map = Map.get()\n"
				"\tmap:setInputEnabled(false)\n"
				"\tlocal player = map:getPlayer()\n"
				"\tif player then\n"
				"\t\tplayer:setInvulnerable(60000)\n"
				"\t\tplayer:setGravityScale(0)\n"
				"\tend\n"
				"end\n";
		if (doc.getScriptLogic().find("function onMapLoaded") == std::string::npos) {
			doc.beginScriptUndo();
			doc.getScriptLogicMutable() += snippet;
			doc.markScriptChanged();
		}
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Adds onMapLoaded that locks input and pins the machine.").c_str());
	ImGui::SameLine();
	if (ImGui::Button(tr("Insert skip-to-finish").c_str())) {
		const char* snippet =
				"function onUpdate(dt)\n"
				"\tlocal map = Map.get()\n"
				"\tif map:isKeyPressed(\"skip\") then\n"
				"\t\tmap:consumeSkip()\n"
				"\t\tmap:finish()\n"
				"\tend\n"
				"end\n";
		if (doc.getScriptLogic().find("function onUpdate") == std::string::npos) {
			doc.beginScriptUndo();
			doc.getScriptLogicMutable() += snippet;
			doc.markScriptChanged();
		}
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Adds onUpdate that ends the cutscene when skip is pressed.").c_str());
	if (ImGui::Button(tr("Insert HUD message").c_str())) {
		const char* snippet =
				"\tmap:message(\"Your text\", 3000)\n";
		doc.beginScriptUndo();
		doc.getScriptLogicMutable() += snippet;
		doc.markScriptChanged();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Inserts map:message in the script (put it inside onMapLoaded / onUpdate).").c_str());
	ImGui::SameLine();
	if (ImGui::Button(tr("Insert spawn villager").c_str())) {
		const char* snippet =
				"\t-- 1-based cave index; type npc-man / npc-woman / npc-grandpa\n"
				"\tlocal npc = map:spawnFriendlyNPC(1, \"npc-man\", false)\n";
		doc.beginScriptUndo();
		doc.getScriptLogicMutable() += snippet;
		doc.markScriptChanged();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Inserts spawnFriendlyNPC. Runtime only — not valid in initMap.").c_str());
}

bool UICaveExpressMapEditorWindow::handleCanvasOverlayInput (float tileW, float tileH) const
{
	MapEditorDocument& doc = ceDocument();
	const float waterY = static_cast<float>(doc.getMapHeight()) - doc.getWaterHeight();
	const float waterPixel = _canvasMinY + waterY * tileH - _panY;
	const float mouseY = ImGui::GetIO().MousePos.y;
	const bool near = _canvasHovered && std::fabs(mouseY - waterPixel) <= 8.0f;
	if (near && ImGui::GetIO().KeyAlt && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		_waterDragging = true;
	if (_waterDragging) {
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			const float gy = (mouseY - _canvasMinY + _panY) / tileH;
			doc.setWaterHeight(clamp(static_cast<float>(doc.getMapHeight()) - gy, 0.0f,
					static_cast<float>(doc.getMapHeight())));
			return true;
		}
		_waterDragging = false;
	}
	return false;
}

}
