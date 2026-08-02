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
	if (ImGui::Button("New"))
		requestAction("new");
	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		if (!_doc->save())
			Log::error(LOG_UI, "Failed to save map");
	}
	ImGui::SameLine();
	if (ImGui::Button("Save & Play")) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		_doc->saveAndPlay();
	}
	ImGui::SameLine();
	if (ImGui::Button("Undo") && _doc->canUndo())
		_doc->undo();
	ImGui::SameLine();
	if (ImGui::Button("Redo") && _doc->canRedo())
		_doc->redo();
	ImGui::SameLine();
	if (ImGui::Button("Fit"))
		fitView();
	ImGui::SameLine();
	if (ImGui::Button("Help"))
		_showHelp = !_showHelp;
	ImGui::SameLine();
	ImGui::TextDisabled("|");
	ImGui::SameLine();
	if (ImGui::RadioButton("Paint", _doc->getTool() == IMapEditorDocument::Tool::Paint))
		_doc->setTool(IMapEditorDocument::Tool::Paint);
	ImGui::SameLine();
	if (ImGui::RadioButton("Erase", _doc->getTool() == IMapEditorDocument::Tool::Erase))
		_doc->setTool(IMapEditorDocument::Tool::Erase);
	ImGui::SameLine();
	if (ImGui::RadioButton("Pick", _doc->getTool() == IMapEditorDocument::Tool::Pick))
		_doc->setTool(IMapEditorDocument::Tool::Pick);
	ImGui::SameLine();
	ImGui::Text("%s%s", _doc->getFileName().c_str(), _doc->isDirty() ? " *" : "");
}

void UIMapEditorWindow::drawTilesPanel () const
{
	if (_paletteTheme != &_doc->getTheme())
		rebuildPalettes();
	ImGui::InputText("Filter", _tileFilter, sizeof(_tileFilter));
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
	ImGui::InputText("Filter", _entityFilter, sizeof(_entityFilter));
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
	for (int layer = LAYER_BACKGROUND; layer < LAYER_MAX; ++layer) {
		bool active = _doc->isLayerActive(layer);
		if (ImGui::Checkbox(MapEditorLayerNames[layer], &active))
			_doc->toggleLayer(static_cast<MapEditorLayer>(layer));
	}
	bool grid = _doc->isRenderGrid();
	if (ImGui::Checkbox("Grid", &grid))
		_doc->toggleGrid();
}

void UIMapEditorWindow::drawPropertiesPanel () const
{
	if (ImGui::InputText("File", _fileNameBuf, sizeof(_fileNameBuf)))
		_doc->setFileName(_fileNameBuf);
	if (ImGui::InputText("Title", _mapTitleBuf, sizeof(_mapTitleBuf)))
		_doc->setMapName(_mapTitleBuf);

	int w = _doc->getMapWidth();
	int h = _doc->getMapHeight();
	if (ImGui::InputInt("Width", &w))
		_doc->resizeMap(w, _doc->getMapHeight());
	if (ImGui::InputInt("Height", &h))
		_doc->resizeMap(_doc->getMapWidth(), h);

	if (_doc->supportsEmitterParams()) {
		int amount = _doc->getEmitterAmount();
		if (ImGui::InputInt("Emitter amount", &amount))
			_doc->setEmitterAmount(std::max(0, amount));
		int delay = _doc->getEmitterDelay();
		if (ImGui::InputInt("Emitter delay", &delay))
			_doc->setEmitterDelay(std::max(0, delay));
	}

	if (const MapEditorTileItem* highlight = _doc->getHighlightItem()) {
		ImGui::Separator();
		ImGui::Text("Selected: %s", highlight->def->id.c_str());
		ImGui::Text("Grid: %.1f, %.1f", highlight->gridX, highlight->gridY);
		if (highlight->entityType)
			ImGui::Text("Entity: %s", highlight->entityType->name.c_str());
	}

	ImGui::Separator();
	if (ImGui::Button("+W")) _doc->shift(1, 0);
	ImGui::SameLine();
	if (ImGui::Button("-W")) _doc->shift(-1, 0);
	ImGui::SameLine();
	if (ImGui::Button("+H")) _doc->shift(0, 1);
	ImGui::SameLine();
	if (ImGui::Button("-H")) _doc->shift(0, -1);
}

void UIMapEditorWindow::drawMapsPanel () const
{
	ImGui::InputText("Filter", _mapFilter, sizeof(_mapFilter));
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
	ImGui::TextUnformatted("Map Editor");
	ImGui::Separator();
	ImGui::BulletText("LMB: paint / place");
	ImGui::BulletText("RMB: erase");
	ImGui::BulletText("MMB or Space+LMB: pan");
	ImGui::BulletText("Wheel: zoom toward cursor");
	ImGui::BulletText("Space (on canvas): rotate brush");
	ImGui::BulletText("Ctrl+S save, Ctrl+Z/Y undo/redo");
	ImGui::BulletText("F fit view, G toggle grid, F1 help");
	ImGui::BulletText("Delete: remove selection");
	drawHelpExtras();
}

void UIMapEditorWindow::drawConfirmModal () const
{
	if (_showConfirm)
		ImGui::OpenPopup("Unsaved changes");
	if (ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("You have unsaved changes. Continue?");
		if (ImGui::Button("Discard")) {
			executePendingAction();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
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
		_doc->setSelectedGrid(std::floor((mouse.x - _canvasMinX + _panX) / tileW),
				std::floor((mouse.y - _canvasMinY + _panY) / tileH));
		_doc->setHighlightFromSelection();

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
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				_doc->paintAtSelection(true, true);
			else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
				_doc->paintAtSelection(true, false);
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				_doc->eraseAtSelection(true);
			else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
				_doc->eraseAtSelection(false);
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && !ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
				_doc->pickAtSelection();
		}
	}

	renderMapIntoCanvas(ImGui::GetWindowDrawList());
	ImGui::EndChild();
}

void UIMapEditorWindow::render (int x, int y) const
{
	const float pw = static_cast<float>(getRenderWidth(false));
	const float ph = static_cast<float>(getRenderHeight(false));
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(getRenderX(false)), static_cast<float>(getRenderY(false))), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(pw, ph), ImGuiCond_Always);
	ImGui::Begin("Map Editor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);

	handleHotkeys();
	drawToolbar();
	ImGui::Separator();

	const float leftWidth = 260.0f;
	const float rightWidth = 280.0f;
	ImGui::BeginChild("left", ImVec2(leftWidth, 0), true);
	if (ImGui::BeginTabBar("left_tabs")) {
		if (ImGui::BeginTabItem("Tiles")) {
			drawTilesPanel();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Entities")) {
			drawEntitiesPanel();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Maps")) {
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
		if (ImGui::BeginTabItem("Properties")) {
			drawPropertiesPanel();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Layers")) {
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
