#include "ui/editor/UIMapEditorWindow.h"
#include "ui/editor/ImGuiTextureDraw.h"
#include "imgui.h"
#include "ui/UI.h"
#include "common/Log.h"
#include "common/String.h"
#include "common/MapSettings.h"
#include "common/ThemeType.h"
#include "common/Animation.h"
#include "sprites/Sprite.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace {
const float ZOOM_MIN = 0.15f;
const float ZOOM_MAX = 8.0f;
const float ZOOM_STEP = 0.1f;

bool passesFilter (const std::string& text, const char* filter)
{
	if (filter == nullptr || filter[0] == '\0')
		return true;
	std::string lower = text;
	std::string needle = filter;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
	return lower.find(needle) != std::string::npos;
}
}

UIMapEditorWindow::UIMapEditorWindow (IFrontend* frontend, std::unique_ptr<IMapEditorDocument> doc) :
		UIWindow(UI_WINDOW_EDITOR, frontend, WINDOW_FLAG_FULLSCREEN | WINDOW_FLAG_ROOT),
		_doc(std::move(doc)), _frontendPtr(frontend)
{
	_doc->registerCommands();
	_tileRefWidth = std::max(1, UI::get().loadTexture("tile-reference")->getWidth());
	_zoom = 32.0f / static_cast<float>(_tileRefWidth);
	std::strncpy(_fileNameBuf, _doc->getFileName().c_str(), sizeof(_fileNameBuf) - 1);
	std::strncpy(_mapTitleBuf, _doc->getMapName().c_str(), sizeof(_mapTitleBuf) - 1);
	rebuildPalettes();
	_musicFile.clear();
}

UIMapEditorWindow::~UIMapEditorWindow ()
{
	_doc->unregisterCommands();
}

bool UIMapEditorWindow::onPush ()
{
	std::strncpy(_fileNameBuf, _doc->getFileName().c_str(), sizeof(_fileNameBuf) - 1);
	std::strncpy(_mapTitleBuf, _doc->getMapName().c_str(), sizeof(_mapTitleBuf) - 1);
	rebuildPalettes();
	fitView();
	return UIWindow::onPush();
}

bool UIMapEditorWindow::onPop ()
{
	if (_doc->isDirty()) {
		_showConfirm = true;
		_confirmAction = "pop";
		return false;
	}
	return UIWindow::onPop();
}

void UIMapEditorWindow::rebuildPalettes () const
{
	_paletteTheme = &_doc->getTheme();
	_tilePalette.clear();
	_entityPalette.clear();
	_doc->collectTilePalette(_tilePalette);
	_doc->collectEntityPalette(_entityPalette);
}

float UIMapEditorWindow::tileWidth () const
{
	return static_cast<float>(_tileRefWidth) * _zoom;
}

float UIMapEditorWindow::tileHeight () const
{
	return std::max(1.0f, static_cast<float>(_tileRefWidth) * _zoom + 0.5f);
}

void UIMapEditorWindow::fitView () const
{
	const float cw = std::max(1.0f, _canvasMaxX - _canvasMinX);
	const float ch = std::max(1.0f, _canvasMaxY - _canvasMinY);
	if (cw < 2.0f || ch < 2.0f) {
		_zoom = 32.0f / static_cast<float>(_tileRefWidth);
		_panX = _panY = 0.0f;
		return;
	}
	const float zx = cw / (static_cast<float>(_doc->getMapWidth()) * static_cast<float>(_tileRefWidth));
	const float zy = ch / (static_cast<float>(_doc->getMapHeight()) * static_cast<float>(_tileRefWidth));
	_zoom = std::max(ZOOM_MIN, std::min(zx, zy) * 0.95f);
	_panX = _panY = 0.0f;
}

bool UIMapEditorWindow::requestAction (const char* action) const
{
	if (_doc->isDirty()) {
		_showConfirm = true;
		_confirmAction = action;
		return false;
	}
	_confirmAction = action;
	executePendingAction();
	return true;
}

void UIMapEditorWindow::executePendingAction () const
{
	const std::string action = _confirmAction;
	_confirmAction.clear();
	_showConfirm = false;
	if (action == "new") {
		_doc->newMap();
		std::strncpy(_fileNameBuf, _doc->getFileName().c_str(), sizeof(_fileNameBuf) - 1);
		std::strncpy(_mapTitleBuf, _doc->getMapName().c_str(), sizeof(_mapTitleBuf) - 1);
		rebuildPalettes();
		fitView();
	} else if (action == "pop") {
		UI::get().pop();
	} else if (string::startsWith(action, "load:")) {
		_doc->load(action.substr(5));
		std::strncpy(_fileNameBuf, _doc->getFileName().c_str(), sizeof(_fileNameBuf) - 1);
		std::strncpy(_mapTitleBuf, _doc->getMapName().c_str(), sizeof(_mapTitleBuf) - 1);
		rebuildPalettes();
		fitView();
	}
}

void UIMapEditorWindow::handleHotkeys () const
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput)
		return;

	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		if (!_doc->save())
			Log::error(LOG_UI, "Failed to save map");
	}
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
		_doc->undo();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false))
		_doc->redo();
	if (ImGui::IsKeyPressed(ImGuiKey_F1, false))
		_showHelp = !_showHelp;
	if (ImGui::IsKeyPressed(ImGuiKey_G, false))
		_doc->toggleGrid();
	if (ImGui::IsKeyPressed(ImGuiKey_Space, false) && _canvasHovered)
		_doc->rotateBrush();
	if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) || ImGui::IsKeyPressed(ImGuiKey_Backspace, false))
		_doc->deleteSelection();
	if (ImGui::IsKeyPressed(ImGuiKey_F, false))
		fitView();
	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
		if (_showHelp)
			_showHelp = false;
		else if (_showConfirm) {
			_showConfirm = false;
			_confirmAction.clear();
		} else
			UI::get().pop();
	}
}

void UIMapEditorWindow::drawToolbar () const
{
	if (ImGui::Button(tr("New").c_str()))
		requestAction("new");
	ImGui::SameLine();
	if (ImGui::Button(tr("Save").c_str())) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		if (!_doc->save())
			Log::error(LOG_UI, "Failed to save map");
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Save & Go").c_str())) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		_doc->saveAndPlay();
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Undo").c_str()) && _doc->canUndo())
		_doc->undo();
	ImGui::SameLine();
	if (ImGui::Button(tr("Redo").c_str()) && _doc->canRedo())
		_doc->redo();
	ImGui::SameLine();
	if (ImGui::Button(tr("Fit").c_str()))
		fitView();
	ImGui::SameLine();
	if (ImGui::Button(tr("Help").c_str()))
		_showHelp = !_showHelp;
	ImGui::SameLine();
	ImGui::TextDisabled("|");
	ImGui::SameLine();
	if (ImGui::RadioButton(tr("Place tile").c_str(), _doc->getTool() == IMapEditorDocument::Tool::Paint))
		_doc->setTool(IMapEditorDocument::Tool::Paint);
	ImGui::SameLine();
	if (ImGui::RadioButton(tr("Remove tile").c_str(), _doc->getTool() == IMapEditorDocument::Tool::Erase))
		_doc->setTool(IMapEditorDocument::Tool::Erase);
	ImGui::SameLine();
	if (ImGui::RadioButton(tr("Select tile").c_str(), _doc->getTool() == IMapEditorDocument::Tool::Pick))
		_doc->setTool(IMapEditorDocument::Tool::Pick);
	ImGui::SameLine();
	ImGui::Text("%s%s", _doc->getFileName().c_str(), _doc->isDirty() ? " *" : "");
}

void UIMapEditorWindow::drawTilesPanel () const
{
	if (_paletteTheme != &_doc->getTheme())
		rebuildPalettes();
	ImGui::InputText(tr("Filter").c_str(), _tileFilter, sizeof(_tileFilter));
	ImGui::BeginChild("tiles_grid", ImVec2(0, 0), true);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	const float cell = 48.0f;
	const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (cell + 4.0f)));
	int col = 0;
	for (const SpriteDefPtr& sprite : _tilePalette) {
		if (!passesFilter(sprite->id, _tileFilter))
			continue;
		if (col > 0)
			ImGui::SameLine();
		ImGui::PushID(sprite->id.c_str());
		const bool selected = _doc->getActiveSprite() && _doc->getActiveSprite()->id == sprite->id && _doc->getActiveEntityType() == nullptr;
		if (ImGui::Selectable("##tile", selected, 0, ImVec2(cell, cell)))
			_doc->setSprite(sprite);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", sprite->id.c_str());
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		const SpritePtr& s = UI::get().loadSprite(sprite->id);
		mapEditorAddSprite(drawList, _frontendPtr, s,
				ImVec2(min.x + 4.0f, min.y + 4.0f), ImVec2(max.x - 4.0f, max.y - 4.0f), 1.0f, sprite->angle);
		ImGui::PopID();
		col = (col + 1) % columns;
	}
	ImGui::EndChild();
}

void UIMapEditorWindow::drawEntitiesPanel () const
{
	ImGui::InputText(tr("Filter").c_str(), _entityFilter, sizeof(_entityFilter));
	ImGui::BeginChild("entities_list", ImVec2(0, 0), true);
	for (const EntityType* type : _entityPalette) {
		if (!passesFilter(type->name, _entityFilter))
			continue;
		if (ImGui::Selectable(type->name.c_str(), _doc->getActiveEntityType() == type))
			_doc->setEmitterEntity(*type);
	}
	ImGui::EndChild();
}

void UIMapEditorWindow::drawLayersPanel () const
{
	auto layerLabel = [] (int layer) -> std::string {
		switch (layer) {
		case LAYER_BACKGROUND: return tr("background");
		case LAYER_SOLID: return tr("solid");
		case LAYER_FOREGROUND: return tr("foreground");
		case LAYER_DECORATION: return tr("decoration");
		case LAYER_EMITTER: return tr("emitter");
		default: return tr("none");
		}
	};
	for (int layer = LAYER_BACKGROUND; layer < LAYER_MAX; ++layer) {
		bool active = _doc->isLayerActive(layer);
		const std::string label = layerLabel(layer);
		if (ImGui::Checkbox(label.c_str(), &active))
			_doc->toggleLayer(static_cast<MapEditorLayer>(layer));
	}
	bool grid = _doc->isRenderGrid();
	if (ImGui::Checkbox(tr("Show Grid").c_str(), &grid))
		_doc->toggleGrid();
}

void UIMapEditorWindow::drawPropertiesPanel () const
{
	if (ImGui::InputText(tr("File").c_str(), _fileNameBuf, sizeof(_fileNameBuf)))
		_doc->setFileName(_fileNameBuf);
	if (ImGui::InputText(tr("Title").c_str(), _mapTitleBuf, sizeof(_mapTitleBuf)))
		_doc->setMapName(_mapTitleBuf);

	int w = _doc->getMapWidth();
	int h = _doc->getMapHeight();
	if (ImGui::InputInt(tr("Width").c_str(), &w))
		_doc->resizeMap(w, _doc->getMapHeight());
	if (ImGui::InputInt(tr("Height").c_str(), &h))
		_doc->resizeMap(_doc->getMapWidth(), h);

	if (_doc->supportsEmitterParams()) {
		int amount = _doc->getEmitterAmount();
		if (ImGui::InputInt(tr("Emitter amount").c_str(), &amount))
			_doc->setEmitterAmount(std::max(0, amount));
		int delay = _doc->getEmitterDelay();
		if (ImGui::InputInt(tr("Emitter delay").c_str(), &delay))
			_doc->setEmitterDelay(std::max(0, delay));
	}

	// Fixed-height selection block so the panel does not reflow when selection changes.
	ImGui::Separator();
	const float selectionHeight = ImGui::GetTextLineHeightWithSpacing() * 3.25f;
	ImGui::BeginChild("selection_info", ImVec2(0.0f, selectionHeight), false, ImGuiWindowFlags_NoScrollbar);
	if (const MapEditorTileItem* highlight = _doc->getHighlightItem()) {
		ImGui::TextWrapped("%s", highlight->def->id.c_str());
		ImGui::Text("%s: %.1f, %.1f", tr("Grid").c_str(), highlight->gridX, highlight->gridY);
		if (highlight->entityType)
			ImGui::Text("%s: %s", tr("Entity").c_str(), highlight->entityType->name.c_str());
	} else {
		ImGui::TextDisabled("%s", tr("Select a tile on the map").c_str());
		ImGui::TextDisabled(" ");
		ImGui::TextDisabled(" ");
	}
	ImGui::EndChild();

	ImGui::Separator();
	if (ImGui::Button("+W")) _doc->shift(1, 0);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tr("Increase map width").c_str());
	ImGui::SameLine();
	if (ImGui::Button("-W")) _doc->shift(-1, 0);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tr("Decrease map width").c_str());
	ImGui::SameLine();
	if (ImGui::Button("+H")) _doc->shift(0, 1);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tr("Increase map height").c_str());
	ImGui::SameLine();
	if (ImGui::Button("-H")) _doc->shift(0, -1);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tr("Decrease map height").c_str());
}

void UIMapEditorWindow::drawMapsPanel () const
{
	ImGui::InputText(tr("Filter").c_str(), _mapFilter, sizeof(_mapFilter));
	ImGui::BeginChild("maps_list", ImVec2(0, 0), true);
	for (const auto& entry : _doc->getMapManager().getMapsByWildcard("*")) {
		if (!passesFilter(entry.first, _mapFilter))
			continue;
		if (ImGui::Selectable(entry.first.c_str(), entry.first == _doc->getFileName()))
			requestAction(("load:" + entry.first).c_str());
	}
	ImGui::EndChild();
}

void UIMapEditorWindow::drawHelpPanel () const
{
	ImGui::TextUnformatted(tr("Map Editor").c_str());
	ImGui::Separator();
	ImGui::BulletText("%s", tr("LMB: paint / place (also selects)").c_str());
	ImGui::BulletText("%s", tr("Select tool or Shift+LMB: select / pick tile").c_str());
	ImGui::BulletText("%s", tr("RMB: erase").c_str());
	ImGui::BulletText("%s", tr("MMB click: pick, MMB drag or Space+LMB: pan").c_str());
	ImGui::BulletText("%s", tr("Wheel: zoom toward cursor").c_str());
	ImGui::BulletText("%s", tr("Space (on canvas): rotate brush").c_str());
	ImGui::BulletText("%s", tr("Ctrl+S save, Ctrl+Z/Y undo/redo").c_str());
	ImGui::BulletText("%s", tr("F fit view, G toggle grid, F1 help").c_str());
	ImGui::BulletText("%s", tr("Delete: remove selection").c_str());
	ImGui::BulletText("%s", tr("Properties edit the selected tile, not the hovered one").c_str());
	drawHelpExtras();
}

void UIMapEditorWindow::drawConfirmModal () const
{
	const std::string popupTitle = tr("Unsaved changes") + "###unsaved";
	if (_showConfirm)
		ImGui::OpenPopup(popupTitle.c_str());
	if (ImGui::BeginPopupModal(popupTitle.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted(tr("You have unsaved changes. Continue?").c_str());
		if (ImGui::Button(tr("Quit without saving").c_str())) {
			executePendingAction();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(tr("Cancel").c_str())) {
			_showConfirm = false;
			_confirmAction.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void UIMapEditorWindow::renderSprite (ImDrawList* drawList, const MapEditorTileItem& item, float originX, float originY,
		float tileW, float tileH, float alpha) const
{
	if (!item.def)
		return;
	const SpritePtr& sprite = UI::get().loadSprite(item.def->id);
	const vec2 size = item.getSize(false);
	const float rx = originX + (item.gridX + item.getX(false)) * tileW - _panX;
	const float ry = originY + (item.gridY + item.getY(false)) * tileH - _panY;
	const float rw = size.x * tileW;
	const float rh = size.y * tileH;
	mapEditorAddSprite(drawList, _frontendPtr, sprite, ImVec2(rx, ry), ImVec2(rx + rw, ry + rh), alpha, item.angle);
}

void UIMapEditorWindow::renderMapIntoCanvas (ImDrawList* drawList) const
{
	const float x = _canvasMinX;
	const float y = _canvasMinY;
	const float w = _canvasMaxX - _canvasMinX;
	const float h = _canvasMaxY - _canvasMinY;
	if (drawList == nullptr || w <= 0.0f || h <= 0.0f)
		return;

	drawList->PushClipRect(ImVec2(x, y), ImVec2(x + w, y + h), true);
	drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(40, 40, 40, 255));

	const float tileW = tileWidth();
	const float tileH = tileHeight();
	const float startGX = _panX / tileW;
	const float startGY = _panY / tileH;
	const int visibleW = static_cast<int>(w / tileW) + 3;
	const int visibleH = static_cast<int>(h / tileH) + 3;

	for (const MapEditorTileItem& item : _doc->getTiles()) {
		if (!_doc->isLayerActive(item.layer))
			continue;
		if (item.gridX < startGX - 2 || item.gridY < startGY - 2)
			continue;
		if (item.gridX >= startGX + visibleW || item.gridY >= startGY + visibleH)
			continue;
		renderSprite(drawList, item, x, y, tileW, tileH);
		if (_doc->getHighlightItem() != nullptr && *_doc->getHighlightItem() == item) {
			const float hx = x + (item.gridX + item.getX(false)) * tileW - _panX;
			const float hy = y + (item.gridY + item.getY(false)) * tileH - _panY;
			const vec2 size = item.getSize(false);
			drawList->AddRect(ImVec2(hx, hy), ImVec2(hx + size.x * tileW, hy + size.y * tileH), IM_COL32(255, 255, 0, 255));
		}
	}

	for (const IMap::StartPosition& position : _doc->getStartPositions()) {
		const SpriteDefPtr& def = SpriteDefinition::get().getFromEntityType(_doc->getPlayerEntityType(), getPlayerAnimation());
		if (!def)
			continue;
		MapEditorTileItem item;
		item.def = def;
		item.gridX = string::toFloat(position._x);
		item.gridY = string::toFloat(position._y);
		item.mapTile = false;
		renderSprite(drawList, item, x, y, tileW, tileH);
	}

	renderCanvasOverlay(drawList, x, y, tileW, tileH);

	if (_doc->isRenderGrid()) {
		const ImU32 gridColor = IM_COL32(80, 80, 80, 160);
		for (int gx = 0; gx <= _doc->getMapWidth(); ++gx) {
			const float px = x + gx * tileW - _panX;
			drawList->AddLine(ImVec2(px, y - _panY), ImVec2(px, y + _doc->getMapHeight() * tileH - _panY), gridColor);
		}
		for (int gy = 0; gy <= _doc->getMapHeight(); ++gy) {
			const float py = y + gy * tileH - _panY;
			drawList->AddLine(ImVec2(x - _panX, py), ImVec2(x + _doc->getMapWidth() * tileW - _panX, py), gridColor);
		}
	}

	if (_canvasHovered && _doc->getActiveSprite()) {
		MapEditorTileItem ghost;
		ghost.def = _doc->getActiveSprite();
		ghost.entityType = _doc->getActiveEntityType();
		ghost.gridX = _doc->getSelectedGridX();
		ghost.gridY = _doc->getSelectedGridY();
		ghost.angle = _doc->getActiveAngle();
		ghost.mapTile = _doc->getActiveEntityType() == nullptr;
		renderSprite(drawList, ghost, x, y, tileW, tileH, 0.55f);
		const float hx = x + (ghost.gridX + ghost.getX(false)) * tileW - _panX;
		const float hy = y + (ghost.gridY + ghost.getY(false)) * tileH - _panY;
		const vec2 size = ghost.getSize(false);
		drawList->AddRect(ImVec2(hx, hy), ImVec2(hx + size.x * tileW, hy + size.y * tileH), IM_COL32(0, 255, 0, 255));
	}

	drawList->PopClipRect();
}

void UIMapEditorWindow::drawCanvas () const
{
	ImGui::BeginChild("canvas", ImVec2(0, 0), false,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);
	const ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
	_canvasMinX = canvasPos.x;
	_canvasMinY = canvasPos.y;
	_canvasMaxX = canvasPos.x + canvasSize.x;
	_canvasMaxY = canvasPos.y + canvasSize.y;

	ImGui::InvisibleButton("canvas_btn", canvasSize,
			ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
	_canvasHovered = ImGui::IsItemHovered();

	const float tileW = tileWidth();
	const float tileH = tileHeight();
	if (_canvasHovered) {
		const ImVec2 mouse = ImGui::GetIO().MousePos;
		// Cursor grid follows the mouse for painting; properties use the stable click selection.
		_doc->setSelectedGrid(std::floor((mouse.x - _canvasMinX + _panX) / tileW),
				std::floor((mouse.y - _canvasMinY + _panY) / tileH));

		const bool space = ImGui::IsKeyDown(ImGuiKey_Space);
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || (space && ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
			_panning = true;
			_panX -= ImGui::GetIO().MouseDelta.x;
			_panY -= ImGui::GetIO().MouseDelta.y;
		} else {
			_panning = false;
		}

		const float wheel = ImGui::GetIO().MouseWheel;
		if (wheel != 0.0f) {
			const float oldZoom = _zoom;
			_zoom = std::max(ZOOM_MIN, std::min(ZOOM_MAX, _zoom + wheel * ZOOM_STEP * _zoom));
			const float ratio = _zoom / oldZoom;
			const float mx = mouse.x - _canvasMinX;
			const float my = mouse.y - _canvasMinY;
			_panX = (_panX + mx) * ratio - mx;
			_panY = (_panY + my) * ratio - my;
		}

		if (!_panning && !space) {
			const bool shift = ImGui::GetIO().KeyShift;
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				if (shift) {
					_doc->pickAtSelection();
				} else {
					_doc->beginUndoStroke();
					_doc->paintAtSelection(true, false);
				}
			} else if (!shift && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				_doc->paintAtSelection(true, false);
			}
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				_doc->beginUndoStroke();
				_doc->eraseAtSelection(false);
			} else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
				_doc->eraseAtSelection(false);
			}
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && !ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
				_doc->pickAtSelection();
		}
	}

	// Commit paint/erase strokes even if the cursor left the canvas before release.
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		_doc->endUndoStroke();

	renderMapIntoCanvas(ImGui::GetWindowDrawList());
	ImGui::EndChild();
}

void UIMapEditorWindow::render (int x, int y) const
{
	const float pw = static_cast<float>(getRenderWidth(false));
	const float ph = static_cast<float>(getRenderHeight(false));
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(getRenderX(false)), static_cast<float>(getRenderY(false))), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(pw, ph), ImGuiCond_Always);
	ImGui::Begin(tr("Map Editor").c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);

	handleHotkeys();
	drawToolbar();
	ImGui::Separator();

	const float leftWidth = 260.0f;
	const float rightWidth = 280.0f;
	ImGui::BeginChild("left", ImVec2(leftWidth, 0), true);
	if (ImGui::BeginTabBar("left_tabs")) {
		if (ImGui::BeginTabItem(tr("Tiles").c_str())) {
			drawTilesPanel();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(tr("Entities").c_str())) {
			drawEntitiesPanel();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(tr("Maps").c_str())) {
			drawMapsPanel();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("center", ImVec2(-rightWidth - 8.0f, 0), false);
	drawCanvas();
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("right", ImVec2(rightWidth, 0), true);
	if (ImGui::BeginTabBar("right_tabs")) {
		if (ImGui::BeginTabItem(tr("Properties").c_str())) {
			drawPropertiesPanel();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(tr("Layers").c_str())) {
			drawLayersPanel();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	if (_showHelp) {
		ImGui::Separator();
		drawHelpPanel();
	}
	ImGui::EndChild();

	drawConfirmModal();
	ImGui::End();
}
