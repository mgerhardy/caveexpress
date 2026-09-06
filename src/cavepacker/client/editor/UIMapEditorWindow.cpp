#include "UIMapEditorWindow.h"
#include "cavepacker/client/editor/MapEditorDocument.h"
#include "imgui.h"
#include "ui/UI.h"
#include "common/String.h"
#include "common/Log.h"
#include "common/IMap.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

namespace cavepacker {

UIMapEditorWindow::UIMapEditorWindow (IFrontend* frontend, IMapManager& mapManager) :
		::UIMapEditorWindow(frontend, std::unique_ptr<IMapEditorDocument>(new MapEditorDocument(mapManager)))
{
}

void UIMapEditorWindow::drawHelpDocs () const
{
	ImGui::BulletText("docs/cavepacker/EDITOR.md");
	ImGui::BulletText("base/cavepacker/maps/README.mapformat");
}

std::string UIMapEditorWindow::getPlayFromHereTooltip () const
{
	return tr("Save and start at the view center");
}

void UIMapEditorWindow::drawHelpExtras () const
{
	ImGui::Separator();
	ImGui::TextUnformatted(tr("Sokoban tiles").c_str());
	ImGui::BulletText("# wall   @ player   $ package   . target");
	ImGui::BulletText("* package on target   + player on target   (space) floor");
	ImGui::BulletText("%s", tr("Packages sit on floors or targets. Walls replace everything in a cell.").c_str());
	ImGui::BulletText("%s", tr("Auto-tile walls picks rock art from neighboring floors.").c_str());
}

void UIMapEditorWindow::drawPropertiesPanel () const
{
	::UIMapEditorWindow::drawPropertiesPanel();
	MapEditorDocument& doc = sokobanDocument();

	ImGui::Separator();
	ImGui::Text("%s: %i", tr("Walls").c_str(), doc.countWalls());
	ImGui::Text("%s: %i", tr("Floors").c_str(), doc.countGrounds());
	ImGui::Text("%s: %i", tr("Packages").c_str(), doc.countPackages());
	ImGui::SameLine();
	ImGui::Text("%s: %i", tr("Targets").c_str(), doc.countTargets());
	if (doc.countPackages() != doc.countTargets())
		ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.25f, 1.0f), "%s",
				tr("Package count must match target count").c_str());

	const int selX = static_cast<int>(std::floor(doc.getSelectedGridX()));
	const int selY = static_cast<int>(std::floor(doc.getSelectedGridY()));
	ImGui::Text("%s: %c", tr("Sokoban cell").c_str(), doc.cellGlyphAt(selX, selY));

	if (ImGui::Button(tr("Make playable").c_str()))
		doc.makePlayable();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Add a wall border, floors, a package, a target, and a start if they are missing.").c_str());
	ImGui::SameLine();
	if (ImGui::Button(tr("Auto-tile walls").c_str()))
		doc.autoTileWalls(true);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Replace wall sprites using neighboring floors (same rules as map load).").c_str());

	if (ImGui::Button(tr("Check layout").c_str())) {
		_layoutChecked = true;
		_layoutFailure.clear();
		if (!doc.evaluateReachability(_reachablePlayable, _playableCells, _layoutFailure) && _layoutFailure.empty())
			_layoutFailure = tr("Layout");
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Flood-fill from the player: packages and targets must be reachable.").c_str());
	if (_layoutChecked) {
		if (_layoutFailure.empty())
			ImGui::TextColored(ImVec4(0.4f, 0.85f, 0.4f, 1.0f), "%s", tr("Layout ok").c_str());
		else
			ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.3f, 1.0f), "%s: %s", tr("Layout").c_str(),
					_layoutFailure.c_str());
		ImGui::Text("%s: %i / %i", tr("Playable reachable").c_str(), _reachablePlayable, _playableCells);
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
			ImGui::SetTooltip("%s", tr("Save and start at this pad").c_str());
		ImGui::SameLine();
		if (ImGui::Button(tr("Del").c_str()))
			doc.removeStartPosition(i);
		ImGui::PopID();
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
			if (doc.campaignContainsExact(c)) {
				if (ImGui::SmallButton(tr("Remove").c_str())) {
					if (!doc.removeFromCampaign(c))
						Log::error(LOG_UI, "Failed to remove map from campaign");
				}
			} else {
				ImGui::TextDisabled("%s", tr("wildcard").c_str());
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
}

}
