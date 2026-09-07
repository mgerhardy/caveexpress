#include "ui/editor/UIMapEditorWindow.h"
#include "ui/editor/ImGuiTextureDraw.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "ui/UI.h"
#include "common/Log.h"
#include "common/String.h"
#include "common/MapSettings.h"
#include "common/ThemeType.h"
#include "common/Animation.h"
#include "common/FileSystem.h"
#include "common/File.h"
#include "common/LUALibrary.h"
#include "common/SpriteLuaPatcher.h"
#include "sprites/Sprite.h"
#include <SDL.h>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <vector>

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

bool readGameDataFile (const char* name, std::string& out)
{
	const std::string path = FS.getDataDir() + name;
	SDL_RWops* rwops = FS.createRWops(path, "rb");
	FilePtr file = rwops != nullptr ? FilePtr(new File(rwops, path)) : FS.getFile(name);
	if (!file || !file->exists())
		return false;
	void* buf = nullptr;
	const int n = file->read(&buf);
	if (n <= 0 || buf == nullptr) {
		delete[] static_cast<char*>(buf);
		return false;
	}
	out.assign(static_cast<char*>(buf), static_cast<size_t>(n));
	delete[] static_cast<char*>(buf);
	return true;
}

bool writeGameDataFile (const char* name, const std::string& contents)
{
	const std::string path = FS.getDataDir() + name;
	return FS.writeSysFile(path, reinterpret_cast<const unsigned char*>(contents.c_str()), contents.size(), true) >= 0;
}

bool extractSpriteLua (const std::string& lua, const std::string& spriteId, std::string& excerpt)
{
	size_t tableOpen = 0;
	size_t tableClose = 0;
	if (!sprite_lua_patcher::findSpriteTable(lua, spriteId, tableOpen, tableClose))
		return false;
	const std::string needle = "[\"" + spriteId + "\"]";
	const size_t keyStart = lua.rfind(needle, tableOpen);
	if (keyStart == std::string::npos)
		return false;
	excerpt = lua.substr(keyStart, tableClose - keyStart + 1);
	return true;
}

bool replaceSpriteLua (std::string& lua, const std::string& spriteId, const std::string& excerpt)
{
	size_t tableOpen = 0;
	size_t tableClose = 0;
	if (!sprite_lua_patcher::findSpriteTable(lua, spriteId, tableOpen, tableClose))
		return false;
	const std::string needle = "[\"" + spriteId + "\"]";
	const size_t keyStart = lua.rfind(needle, tableOpen);
	if (keyStart == std::string::npos)
		return false;
	lua.replace(keyStart, tableClose - keyStart + 1, excerpt);
	return true;
}

std::string entityLuaKey (const std::string& typeName)
{
	return string::replaceAll(typeName, "-", "");
}

bool extractEntityLua (const std::string& lua, const std::string& typeName, std::string& excerpt)
{
	const std::string key = entityLuaKey(typeName);
	size_t tableOpen = 0;
	size_t tableClose = 0;
	if (!sprite_lua_patcher::findAssignmentTable(lua, key, tableOpen, tableClose))
		return false;
	const size_t keyStart = lua.rfind(key, tableOpen);
	if (keyStart == std::string::npos)
		return false;
	excerpt = lua.substr(keyStart, tableClose - keyStart + 1);
	return true;
}

bool replaceEntityLua (std::string& lua, const std::string& typeName, const std::string& excerpt)
{
	const std::string key = entityLuaKey(typeName);
	size_t tableOpen = 0;
	size_t tableClose = 0;
	if (sprite_lua_patcher::findAssignmentTable(lua, key, tableOpen, tableClose)) {
		const size_t keyStart = lua.rfind(key, tableOpen);
		if (keyStart == std::string::npos)
			return false;
		lua.replace(keyStart, tableClose - keyStart + 1, excerpt);
		return true;
	}
	if (!lua.empty() && lua.back() != '\n')
		lua += '\n';
	lua += excerpt;
	if (!lua.empty() && lua.back() != '\n')
		lua += '\n';
	return true;
}

bool gameDataFileExists (const char* name)
{
	const std::string path = FS.getDataDir() + name;
	SDL_RWops* rwops = FS.createRWops(path, "rb");
	FilePtr file = rwops != nullptr ? FilePtr(new File(rwops, path)) : FS.getFile(name);
	return file && file->exists();
}

void applySpriteFieldsFromLua (const SpriteDefPtr& def, const std::string& excerpt)
{
	if (!def)
		return;
	const size_t brace = excerpt.find('{');
	if (brace == std::string::npos)
		return;
	LUA lua;
	if (!lua.loadBuffer("s = " + excerpt.substr(brace), "sprite-def"))
		return;
	def->width = lua.getFloatValue("s.width", def->width);
	def->height = lua.getFloatValue("s.height", def->height);
	def->fps = lua.getFloatValue("s.fps", def->fps);
	def->friction = lua.getFloatValue("s.friction", def->friction);
	def->restitution = lua.getFloatValue("s.restitution", def->restitution);
	def->rotateable = static_cast<EntityAngle>(lua.getFloatValue("s.rotateable", def->rotateable));
}

void applyEntityFieldsFromLua (const EntityType* type, const std::string& excerpt)
{
	if (type == nullptr)
		return;
	const size_t brace = excerpt.find('{');
	if (brace == std::string::npos)
		return;
	LUA lua;
	if (!lua.loadBuffer("e = " + excerpt.substr(brace), "entity-def"))
		return;
	const_cast<EntityType*>(type)->setSize(lua.getFloatValue("e.width", type->width),
			lua.getFloatValue("e.height", type->height));
}
}

UIMapEditorWindow::UIMapEditorWindow (IFrontend* frontend, std::unique_ptr<IMapEditorDocument> doc) :
		UIWindow(UI_WINDOW_EDITOR, frontend, WINDOW_FLAG_FULLSCREEN | WINDOW_FLAG_ROOT),
		_doc(std::move(doc)), _frontendPtr(frontend)
{
	_doc->registerCommands();
	_tileRefWidth = std::max(1, UI::get().loadTexture("tile-reference")->getWidth());
	_nativeTileRefWidth = _tileRefWidth;
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

void UIMapEditorWindow::centerViewOnGrid (gridCoord x, gridCoord y) const
{
	const float tileW = tileWidth();
	const float tileH = tileHeight();
	const float viewW = std::max(1.0f, _canvasMaxX - _canvasMinX);
	const float viewH = std::max(1.0f, _canvasMaxY - _canvasMinY);
	_panX = x * tileW - viewW * 0.5f + tileW * 0.5f;
	_panY = y * tileH - viewH * 0.5f + tileH * 0.5f;
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
		_doc->discardChanges();
		leaveEditor();
	} else if (string::startsWith(action, "load:")) {
		_doc->load(action.substr(5));
		std::strncpy(_fileNameBuf, _doc->getFileName().c_str(), sizeof(_fileNameBuf) - 1);
		std::strncpy(_mapTitleBuf, _doc->getMapName().c_str(), sizeof(_mapTitleBuf) - 1);
		rebuildPalettes();
		fitView();
	}
}

void UIMapEditorWindow::leaveEditor () const
{
	if (_doc->isDirty()) {
		_showConfirm = true;
		_confirmAction = "pop";
		return;
	}
	// Never mutate the UI stack mid-ImGui/render frame — that crashes on re-entry.
	if (UI::get().canPop())
		UI::get().delayedPop();
	else
		UI::get().delayedPushRoot(UI_WINDOW_MAIN);
}

void UIMapEditorWindow::handleHotkeys () const
{
	ImGuiIO& io = ImGui::GetIO();

	// Allow save while typing in the script editor (WantTextInput is true there).
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		trySave(false);
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) && _showDefinition) {
		_showDefinition = false;
		return;
	}

	if (io.WantTextInput)
		return;

	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C, false))
		_doc->copyRegion();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V, false))
		_doc->pasteAtSelection();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
		_doc->undo();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false))
		_doc->redo();
	if (!io.KeyCtrl) {
		const MapEditorTileItem* highlight = _doc->getHighlightItem();
		const bool fine = io.KeyShift && highlight != nullptr
				&& (highlight->allowsSubTileX() || highlight->entityType != nullptr);
		const gridCoord step = fine ? 0.1f : 1.0f;
		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
			_doc->nudgeSelection(-step, 0);
		if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))
			_doc->nudgeSelection(step, 0);
		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, false))
			_doc->nudgeSelection(0, -step);
		if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, false))
			_doc->nudgeSelection(0, step);
	}
	if (ImGui::IsKeyPressed(ImGuiKey_F1, false))
		_showHelp = !_showHelp;
	if (ImGui::IsKeyPressed(ImGuiKey_G, false))
		_doc->toggleGrid();
	if (ImGui::IsKeyPressed(ImGuiKey_Space, false) && _canvasHovered)
		_doc->rotateSelectionOrBrush();
	if (_canvasHovered
			&& (ImGui::IsKeyPressed(ImGuiKey_Delete, false) || ImGui::IsKeyPressed(ImGuiKey_Backspace, false)))
		_doc->deleteSelection();
	if (ImGui::IsKeyPressed(ImGuiKey_F, false))
		fitView();
	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
		if (_showDefinition)
			_showDefinition = false;
		else if (_showHelp)
			_showHelp = false;
		else if (_showConfirm) {
			_showConfirm = false;
			_confirmAction.clear();
		} else
			leaveEditor();
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
		trySave(false);
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Save to game data").c_str())) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		trySave(true);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s\n%s", tr("Write into the project maps folder").c_str(),
				_doc->getGameDataMapsPath().c_str());
	ImGui::SameLine();
	if (ImGui::Button(tr("Save & Go").c_str())) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		std::vector<std::string> issues;
		_doc->collectValidationIssues(issues);
		if (issues.empty())
			_doc->saveAndPlay();
		else {
			_validationIssues = issues;
			_validationSaveGameData = false;
			_showValidation = true;
			_confirmAction = "play";
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Play from here").c_str())) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		const float tileW = tileWidth();
		const float tileH = tileHeight();
		const gridCoord gx = (_panX + (_canvasMaxX - _canvasMinX) * 0.5f) / tileW;
		const gridCoord gy = (_panY + (_canvasMaxY - _canvasMinY) * 0.5f) / tileH;
		_doc->saveAndPlayFrom(std::floor(gx), std::floor(gy));
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", getPlayFromHereTooltip().c_str());
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
	if (_doc->supportsMapScript()) {
		if (ImGui::Button(tr("Script").c_str()))
			_focusScriptPanel = true;
		ImGui::SameLine();
	}
	if (ImGui::Button(tr("Shapes").c_str())) {
		std::string suggested;
		if (_doc->getHighlightItem() && _doc->getHighlightItem()->def)
			suggested = _doc->getHighlightItem()->def->id;
		else if (_doc->getActiveSprite())
			suggested = _doc->getActiveSprite()->id;
		_shapeEditor.open(suggested);
	}
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
	if (ImGui::RadioButton(tr("Fill").c_str(), _doc->getTool() == IMapEditorDocument::Tool::Fill))
		_doc->setTool(IMapEditorDocument::Tool::Fill);
	ImGui::SameLine();
	if (_doc->getActiveEntityType() != nullptr && _doc->isActiveEntityRight())
		ImGui::TextDisabled("%s 0° R", _doc->getFileName().c_str());
	else if (_doc->getActiveEntityType() != nullptr)
		ImGui::TextDisabled("%s 0° L", _doc->getFileName().c_str());
	else
		ImGui::Text("%s%s  %i°", _doc->getFileName().c_str(), _doc->isDirty() ? " *" : "", _doc->getActiveAngle());
}

void UIMapEditorWindow::drawTilesPanel () const
{
	if (_paletteTheme != &_doc->getTheme())
		rebuildPalettes();
	ImGui::InputText(tr("Filter").c_str(), _tileFilter, sizeof(_tileFilter));
	ImGui::TextDisabled("%s", tr("Right-click a palette item for more actions").c_str());
	ImGui::BeginChild("tiles_grid", ImVec2(0, 0), true);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	const float cell = 48.0f;
	const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (cell + 4.0f)));
	int col = 0;
	for (const SpriteDefPtr& sprite : _tilePalette) {
		if (!sprite || sprite->hasNoTextures() || !passesFilter(sprite->id, _tileFilter))
			continue;
		if (col > 0)
			ImGui::SameLine();
		ImGui::PushID(sprite->id.c_str());
		const bool selected = _doc->getActiveSprite() && _doc->getActiveSprite()->id == sprite->id && _doc->getActiveEntityType() == nullptr;
		if (ImGui::Selectable("##tile", selected, 0, ImVec2(cell, cell)))
			_doc->setSprite(sprite);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", sprite->id.c_str());
		if (ImGui::BeginPopupContextItem("tile_ctx")) {
			drawTileContextMenu(sprite);
			ImGui::EndPopup();
		}
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
	ImGui::TextDisabled("%s", tr("Right-click a palette item for more actions").c_str());
	ImGui::BeginChild("entities_grid", ImVec2(0, 0), true);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	const float cell = 48.0f;
	const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (cell + 4.0f)));
	int col = 0;
	for (const EntityType* type : _entityPalette) {
		if (type == nullptr || !passesFilter(type->name, _entityFilter))
			continue;
		if (col > 0)
			ImGui::SameLine();
		ImGui::PushID(type->name.c_str());
		const bool selected = _doc->getActiveEntityType() == type;
		if (ImGui::Selectable("##entity", selected, 0, ImVec2(cell, cell)))
			_doc->setEmitterEntity(*type);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", type->name.c_str());
		if (ImGui::BeginPopupContextItem("entity_ctx")) {
			drawEntityContextMenu(*type);
			ImGui::EndPopup();
		}
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		const SpriteDefPtr def = _doc->findEntitySprite(*type);
		SpritePtr s;
		if (def && !def->hasNoTextures())
			s = UI::get().loadSprite(def->id);
		if (s)
			mapEditorAddSprite(drawList, _frontendPtr, s,
					ImVec2(min.x + 4.0f, min.y + 4.0f), ImVec2(max.x - 4.0f, max.y - 4.0f), 1.0f, 0);
		else {
			drawList->PushClipRect(min, max, true);
			drawList->AddText(ImVec2(min.x + 2.0f, min.y + cell * 0.5f - 6.0f), IM_COL32_WHITE, type->name.c_str());
			drawList->PopClipRect();
		}
		ImGui::PopID();
		col = (col + 1) % columns;
	}
	ImGui::EndChild();
}

void UIMapEditorWindow::drawTileContextMenu (const SpriteDefPtr& sprite) const
{
	if (!sprite)
		return;
	const int count = _doc->countTilesWithSprite(sprite->id);
	if (ImGui::MenuItem(tr("Use as brush").c_str()))
		_doc->setSprite(sprite);
	if (ImGui::MenuItem(tr("Edit shape").c_str()))
		_shapeEditor.open(sprite->id);
	if (ImGui::MenuItem(tr("Edit tile definition").c_str()))
		openSpriteDefinition(sprite);
	ImGui::Separator();
	char removeLabel[128];
	std::snprintf(removeLabel, sizeof(removeLabel), "%s (%i)", tr("Remove all from map").c_str(), count);
	if (ImGui::MenuItem(removeLabel, nullptr, false, count > 0))
		_doc->removeTilesWithSprite(sprite->id);
	if (ImGui::MenuItem(tr("Go to first on map").c_str(), nullptr, false, count > 0)) {
		gridCoord x = 0.0f;
		gridCoord y = 0.0f;
		if (_doc->findFirstTileWithSprite(sprite->id, x, y)) {
			_doc->setEditMode(IMapEditorDocument::EditMode::Tiles);
			_doc->setSprite(sprite);
			_doc->focusCell(x, y);
			centerViewOnGrid(x, y);
		}
	}
	ImGui::Separator();
	if (ImGui::MenuItem(tr("Copy sprite id").c_str()))
		ImGui::SetClipboardText(sprite->id.c_str());
	ImGui::TextDisabled("%s: %s", tr("Type").c_str(), sprite->type.name.c_str());
	ImGui::TextDisabled("%s: %i", tr("Used on this map").c_str(), count);
}

void UIMapEditorWindow::drawEntityContextMenu (const EntityType& type) const
{
	const int count = _doc->countEntitiesOfType(type);
	const SpriteDefPtr sprite = _doc->findEntitySprite(type);
	if (ImGui::MenuItem(tr("Use as brush").c_str()))
		_doc->setEmitterEntity(type);
	if (sprite && ImGui::MenuItem(tr("Edit shape").c_str()))
		_shapeEditor.open(sprite->id);
	if (gameDataFileExists("entities.lua") && ImGui::MenuItem(tr("Edit entity lua").c_str()))
		openEntityDefinition(type);
	if (sprite && ImGui::MenuItem(tr("Edit sprite definition").c_str()))
		openSpriteDefinition(sprite);
	ImGui::Separator();
	char removeLabel[128];
	std::snprintf(removeLabel, sizeof(removeLabel), "%s (%i)", tr("Remove all from map").c_str(), count);
	if (ImGui::MenuItem(removeLabel, nullptr, false, count > 0))
		_doc->removeEntitiesOfType(type);
	if (ImGui::MenuItem(tr("Go to first on map").c_str(), nullptr, false, count > 0)) {
		gridCoord x = 0.0f;
		gridCoord y = 0.0f;
		if (_doc->findFirstEntityOfType(type, x, y)) {
			_doc->setEditMode(IMapEditorDocument::EditMode::Entities);
			_doc->setEmitterEntity(type);
			_doc->focusCell(x, y);
			centerViewOnGrid(x, y);
		}
	}
	ImGui::Separator();
	if (ImGui::MenuItem(tr("Copy type name").c_str()))
		ImGui::SetClipboardText(type.name.c_str());
	if (sprite)
		ImGui::TextDisabled("%s: %s", tr("Sprite").c_str(), sprite->id.c_str());
	ImGui::TextDisabled("%s: %i", tr("Used on this map").c_str(), count);
}

void UIMapEditorWindow::openSpriteDefinition (const SpriteDefPtr& sprite) const
{
	if (!sprite)
		return;
	_definitionIsEntity = false;
	_definitionId = sprite->id;
	_definitionSprite = sprite;
	_definitionEntity = nullptr;
	_definitionLua.clear();
	_definitionStatus.clear();
	std::string source;
	if (readGameDataFile("sprites.lua", source)) {
		if (!extractSpriteLua(source, sprite->id, _definitionLua))
			_definitionStatus = tr("Not found in sprites.lua");
	} else {
		_definitionStatus = tr("Could not read sprites.lua");
	}
	_showDefinition = true;
}

void UIMapEditorWindow::openEntityDefinition (const EntityType& type) const
{
	_definitionIsEntity = true;
	_definitionId = type.name;
	_definitionEntity = &type;
	_definitionSprite = _doc->findEntitySprite(type);
	_definitionLua.clear();
	_definitionStatus.clear();
	std::string source;
	if (readGameDataFile("entities.lua", source)) {
		if (!extractEntityLua(source, type.name, _definitionLua)) {
			char buf[256];
			std::snprintf(buf, sizeof(buf), "%s = {\n\twidth = %.2f,\n\theight = %.2f,\n}",
					entityLuaKey(type.name).c_str(), type.width, type.height);
			_definitionLua = buf;
			_definitionStatus = tr("Not found in entities.lua");
		}
	} else {
		_definitionStatus = tr("Could not read entities.lua");
	}
	_showDefinition = true;
}

bool UIMapEditorWindow::saveDefinitionEditor () const
{
	if (_definitionLua.empty())
		return false;
	if (_definitionIsEntity) {
		LUA excerptCheck;
		if (!excerptCheck.loadBuffer(_definitionLua, "entities.lua")) {
			_definitionStatus = tr("Invalid Lua");
			return false;
		}
		std::string source;
		if (!readGameDataFile("entities.lua", source)) {
			_definitionStatus = tr("Could not read entities.lua");
			return false;
		}
		if (!replaceEntityLua(source, _definitionId, _definitionLua)) {
			_definitionStatus = tr("Failed to update entities.lua");
			return false;
		}
		LUA fileCheck;
		if (!fileCheck.loadBuffer(source, "entities.lua")) {
			_definitionStatus = tr("Invalid Lua");
			return false;
		}
		if (!writeGameDataFile("entities.lua", source)) {
			_definitionStatus = tr("Failed to write entities.lua");
			return false;
		}
		applyEntityFieldsFromLua(_definitionEntity, _definitionLua);
		_definitionStatus = tr("Wrote entities.lua");
		return true;
	}

	LUA excerptCheck;
	if (!excerptCheck.loadBuffer("sprites = {\n" + _definitionLua + "\n}\n", "sprites.lua")) {
		_definitionStatus = tr("Invalid Lua");
		return false;
	}
	std::string source;
	if (!readGameDataFile("sprites.lua", source)) {
		_definitionStatus = tr("Could not read sprites.lua");
		return false;
	}
	if (!replaceSpriteLua(source, _definitionId, _definitionLua)) {
		_definitionStatus = tr("Failed to update sprites.lua");
		return false;
	}
	LUA fileCheck;
	if (!fileCheck.loadBuffer(source, "sprites.lua")) {
		_definitionStatus = tr("Invalid Lua");
		return false;
	}
	if (!writeGameDataFile("sprites.lua", source)) {
		_definitionStatus = tr("Failed to write sprites.lua");
		return false;
	}
	applySpriteFieldsFromLua(_definitionSprite, _definitionLua);
	_definitionStatus = tr("Wrote sprites.lua");
	return true;
}

void UIMapEditorWindow::drawDefinitionEditor () const
{
	if (!_showDefinition)
		return;
	const std::string title = (_definitionIsEntity ? tr("Entity definition") : tr("Tile definition"))
			+ "###editor_definition";
	if (!ImGui::Begin(title.c_str(), &_showDefinition)) {
		ImGui::End();
		return;
	}
	ImGui::TextUnformatted(_definitionId.c_str());
	if (_definitionIsEntity && _definitionEntity != nullptr) {
		ImGui::Text("%s: %.2f x %.2f", tr("Size").c_str(), _definitionEntity->width, _definitionEntity->height);
		if (_definitionSprite)
			ImGui::Text("%s: %s", tr("Sprite").c_str(), _definitionSprite->id.c_str());
	} else if (_definitionSprite) {
		const SpriteDefPtr& def = _definitionSprite;
		ImGui::Text("%s: %s", tr("Type").c_str(), def->type.name.c_str());
		if (!def->theme.isNone())
			ImGui::Text("%s: %s", tr("Theme").c_str(), def->theme.name.c_str());
		ImGui::Text("%s: %.2f x %.2f", tr("Size").c_str(), def->width, def->height);
		ImGui::Text("%s: %.1f", tr("FPS").c_str(), def->fps);
		ImGui::Text("%s: %s", tr("Has collision shape").c_str(), def->hasShape() ? tr("Yes").c_str() : tr("No").c_str());
	}
	if (_definitionSprite && ImGui::Button(tr("Edit shape").c_str()))
		_shapeEditor.open(_definitionSprite->id);
	ImGui::Separator();
	ImGui::BeginChild("definition_lua", ImVec2(0.0f, -28.0f), true);
	ImGui::InputTextMultiline("##definition_lua_text", &_definitionLua, ImVec2(-FLT_MIN, -FLT_MIN),
			ImGuiInputTextFlags_AllowTabInput);
	ImGui::EndChild();
	if (ImGui::Button((_definitionIsEntity ? tr("Save to entities.lua") : tr("Save to sprites.lua")).c_str()))
		saveDefinitionEditor();
	ImGui::SameLine();
	if (ImGui::Button(tr("Copy").c_str()))
		ImGui::SetClipboardText(_definitionLua.c_str());
	if (!_definitionStatus.empty()) {
		ImGui::SameLine();
		ImGui::TextUnformatted(_definitionStatus.c_str());
	}
	ImGui::End();
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
	if (ImGui::Checkbox(tr("Preview other atlas size").c_str(), &_previewAltAtlas)) {
		_tileRefWidth = _previewAltAtlas ? (_nativeTileRefWidth == 32 ? 16 : 32) : _nativeTileRefWidth;
		fitView();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Change the grid tile size to check alignment against the other texture atlas.").c_str());
}

bool UIMapEditorWindow::beginPropertiesGroup (const char* label, bool defaultOpen) const
{
	return ImGui::CollapsingHeader(label, defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0);
}

void UIMapEditorWindow::drawPropertiesPanel () const
{
	if (beginPropertiesGroup(tr("Map").c_str())) {
		if (ImGui::InputText(tr("File").c_str(), _fileNameBuf, sizeof(_fileNameBuf)))
			_doc->setFileName(_fileNameBuf);
		if (ImGui::InputText(tr("Title").c_str(), _mapTitleBuf, sizeof(_mapTitleBuf)))
			_doc->setMapName(_mapTitleBuf);

		int w = _doc->getMapWidth();
		int h = _doc->getMapHeight();
		if (ImGui::InputInt(tr("Width").c_str(), &w))
			_doc->resizeMap(w, _doc->getMapHeight());
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", tr("Drag a map edge on the canvas to grow or shrink that side. Left and top keep existing tiles in place.").c_str());
		if (ImGui::InputInt(tr("Height").c_str(), &h))
			_doc->resizeMap(_doc->getMapWidth(), h);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", tr("Drag a map edge on the canvas to grow or shrink that side. Left and top keep existing tiles in place.").c_str());
		if (_doc->supportsMapScript()) {
			bool keep = _doc->isPreserveInitMap();
			if (ImGui::Checkbox(tr("Keep handwritten initMap").c_str(), &keep))
				_doc->setPreserveInitMap(keep);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", tr("On save, copy initMap from the existing file instead of regenerating tiles.").c_str());
		}
	}

	if (beginPropertiesGroup(tr("Selection").c_str())) {
		if (_doc->supportsEmitterParams()) {
			MapEditorTileItem* sel = _doc->getHighlightItem();
			if (sel != nullptr && sel->entityType != nullptr) {
				int amount = sel->amount;
				if (ImGui::InputInt(tr("Emitter amount").c_str(), &amount))
					sel->amount = std::max(0, amount);
				int delay = sel->delay;
				if (ImGui::InputInt(tr("Emitter delay").c_str(), &delay))
					sel->delay = std::max(0, delay);
				ImGui::TextDisabled("%s", tr("Editing the selected emitter").c_str());
			} else {
				int amount = _doc->getEmitterAmount();
				if (ImGui::InputInt(tr("Emitter amount").c_str(), &amount))
					_doc->setEmitterAmount(std::max(0, amount));
				int delay = _doc->getEmitterDelay();
				if (ImGui::InputInt(tr("Emitter delay").c_str(), &delay))
					_doc->setEmitterDelay(std::max(0, delay));
				ImGui::TextDisabled("%s", tr("Applies to the next placed emitter").c_str());
			}
		}

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

		if (MapEditorTileItem* sel = _doc->getHighlightItem()) {
			if (sel->allowsSubTileX()) {
				float ex = sel->gridX;
				if (ImGui::InputFloat(tr("X").c_str(), &ex, 0.1f, 1.0f, "%.2f"))
					_doc->setHighlightPosition(ex, sel->gridY);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", tr("Horizontal position in tiles. Drag in Select tool to slide.").c_str());
			}
		}
	}
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
	ImGui::BulletText("%s", tr("Tiles / Entities tab: place and erase only that kind").c_str());
	ImGui::BulletText("%s", tr("LMB: paint / place (also selects)").c_str());
	ImGui::BulletText("%s", tr("Select tool: hover shows the sprite bounds that will be picked").c_str());
	ImGui::BulletText("%s", tr("Select tool or Shift+LMB: select / pick tile").c_str());
	ImGui::BulletText("%s", tr("Select + drag a liane to move it horizontally").c_str());
	ImGui::BulletText("%s", tr("RMB: erase items of the active tab").c_str());
	ImGui::BulletText("%s", tr("MMB click: pick, MMB drag or Space+LMB: pan").c_str());
	ImGui::BulletText("%s", tr("Hover a map edge and drag to resize").c_str());
	ImGui::BulletText("%s", tr("Wheel: zoom toward cursor").c_str());
	ImGui::BulletText("%s", tr("Space (on canvas): rotate selected tile or brush").c_str());
	ImGui::BulletText("%s", tr("Ctrl+S save, Ctrl+Z/Y undo/redo").c_str());
	ImGui::BulletText("%s", tr("F fit view, G toggle grid, F1 help").c_str());
	ImGui::BulletText("%s", tr("Delete: remove selection of the active tab").c_str());
	ImGui::BulletText("%s", tr("Properties edit the selected tile, not the hovered one").c_str());
	if (_doc->supportsMapScript())
		ImGui::BulletText("%s", tr("Script tab: edit Lua (onUpdate/onMapLoaded); Save & Go to test").c_str());
	ImGui::BulletText("%s", tr("Shapes tab: edit polygons/circles and write sprites.lua").c_str());
	ImGui::BulletText("%s", tr("Right-click a palette item for more actions").c_str());
	ImGui::BulletText("%s", tr("Alt+click: pick whatever is on top (any tab)").c_str());
	ImGui::BulletText("%s", tr("Shift+drag (Select): rectangle. Ctrl+C/V copy/paste, arrows nudge").c_str());
	ImGui::BulletText("%s", tr("Shift+arrows: nudge a liane or emitter by 0.1 tiles").c_str());
	ImGui::BulletText("%s", tr("Fill: flood-fill background / same tile").c_str());
	ImGui::Separator();
	ImGui::TextUnformatted(tr("Docs (in the source tree)").c_str());
	drawHelpDocs();
	drawHelpExtras();
}

void UIMapEditorWindow::drawHelpDocs () const
{
	ImGui::BulletText("docs/caveexpress/EDITOR.md");
	ImGui::BulletText("docs/caveexpress/MAPS.md");
	ImGui::BulletText("docs/caveexpress/SPRITES.md");
}

std::string UIMapEditorWindow::getPlayFromHereTooltip () const
{
	return tr("Save and start with the machine at the view center (god mode)");
}

void UIMapEditorWindow::drawScriptEditor () const
{
	if (!_doc->supportsMapScript())
		return;

	if (_focusScriptPanel) {
		ImGui::SetNextWindowFocus();
		_focusScriptPanel = false;
	}
	if (!ImGui::Begin((tr("Script") + "###editor_script").c_str())) {
		ImGui::End();
		return;
	}

	ImGui::TextWrapped("%s", tr("Edit Lua kept across saves (onMapLoaded, onUpdate, intro, helpers). getName and initMap are regenerated from the map data on save unless Keep handwritten initMap is checked.").c_str());
	ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "%s",
			tr("Do not put tiles in this window — initMap is rebuilt from the canvas.").c_str());
	ImGui::Separator();

	if (ImGui::Button(tr("Save").c_str())) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		trySave(false);
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Save & Go").c_str())) {
		_doc->setFileName(_fileNameBuf);
		_doc->setMapName(_mapTitleBuf);
		_doc->saveAndPlay();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s", _doc->isDirty() ? "*" : "");
	ImGui::SameLine();
	bool keep = _doc->isPreserveInitMap();
	if (ImGui::Checkbox(tr("Keep initMap").c_str(), &keep))
		_doc->setPreserveInitMap(keep);

	ImGui::SetNextItemWidth(160.0f);
	ImGui::InputText(tr("Find").c_str(), _scriptFind, sizeof(_scriptFind));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(160.0f);
	ImGui::InputText(tr("Replace").c_str(), _scriptReplace, sizeof(_scriptReplace));
	ImGui::SameLine();
	if (ImGui::Button(tr("Find next").c_str()))
		applyScriptFind(false);
	ImGui::SameLine();
	if (ImGui::Button(tr("Replace all").c_str()))
		applyScriptFind(true);

	drawScriptExtras();
	if (ImGui::CollapsingHeader(tr("Lua API").c_str())) {
		ImGui::BulletText("map:setSetting(key, value)");
		ImGui::BulletText("map:addTile / addCave / addEmitter / addStartPosition");
		ImGui::BulletText("onMapLoaded()  onUpdate(dt)  intro(help)");
		ImGui::BulletText("help:headline / text / entity / beginRow / bar");
		ImGui::BulletText("map:setInputEnabled / finish / consumeSkip / message");
		ImGui::BulletText("map:spawnFriendlyNPC / spawnPackage / addTileRuntime");
		ImGui::BulletText("player:setGravityScale / setInvulnerable / setAnimation");
		ImGui::TextDisabled("%s", tr("See docs/caveexpress/MAPS.md").c_str());
	}

	const std::string& script = _doc->getScriptLogic();
	int lines = 1;
	for (char c : script) {
		if (c == '\n')
			++lines;
	}
	ImGui::TextDisabled("%s: %i", tr("Lines").c_str(), lines);

	ImGui::BeginChild("script_body", ImVec2(0, 0), true);
	const float lineCol = 48.0f;
	ImGui::BeginChild("script_lines", ImVec2(lineCol, 0), false, ImGuiWindowFlags_NoScrollbar);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.6f, 1.0f));
	for (int i = 1; i <= lines; ++i)
		ImGui::Text("%i", i);
	ImGui::PopStyleColor();
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::InputTextMultiline("##map_script", &_doc->getScriptLogicMutable(),
			ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_AllowTabInput)) {
		if (ImGui::IsItemActivated())
			_doc->beginScriptUndo();
		_doc->markScriptChanged();
	} else if (ImGui::IsItemActivated()) {
		_doc->beginScriptUndo();
	}
	if (ImGui::IsItemDeactivated())
		_doc->endScriptUndo();
	ImGui::EndChild();

	ImGui::End();
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

bool UIMapEditorWindow::trySave (bool toGameData) const
{
	std::vector<std::string> issues;
	_doc->collectValidationIssues(issues);
	if (!issues.empty()) {
		_validationIssues = issues;
		_validationSaveGameData = toGameData;
		_showValidation = true;
		_confirmAction = toGameData ? "save-gamedata" : "save";
		return false;
	}
	const bool ok = toGameData ? _doc->saveToGameData() : _doc->save();
	if (!ok)
		Log::error(LOG_UI, "Failed to save map");
	return ok;
}

void UIMapEditorWindow::drawValidationModal () const
{
	const std::string popupTitle = tr("Map validation") + "###mapvalid";
	if (_showValidation)
		ImGui::OpenPopup(popupTitle.c_str());
	if (ImGui::BeginPopupModal(popupTitle.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted(tr("These checks would fail CampaignTest / play:").c_str());
		for (const std::string& issue : _validationIssues)
			ImGui::BulletText("%s", issue.c_str());
		if (ImGui::Button(tr("Save anyway").c_str())) {
			bool ok = true;
			if (_confirmAction == "play")
				ok = _doc->saveAndPlay();
			else if (_validationSaveGameData)
				ok = _doc->saveToGameData();
			else
				ok = _doc->save();
			if (!ok)
				Log::error(LOG_UI, "Failed to save map");
			_showValidation = false;
			_confirmAction.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(tr("Cancel").c_str())) {
			_showValidation = false;
			_confirmAction.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void UIMapEditorWindow::applyScriptFind (bool replaceAll) const
{
	if (_scriptFind[0] == '\0')
		return;
	std::string& text = _doc->getScriptLogicMutable();
	if (replaceAll) {
		_doc->beginScriptUndo();
		size_t pos = 0;
		int count = 0;
		while ((pos = text.find(_scriptFind, pos)) != std::string::npos) {
			text.replace(pos, std::strlen(_scriptFind), _scriptReplace);
			pos += std::strlen(_scriptReplace);
			++count;
		}
		if (count > 0)
			_doc->markScriptChanged();
		_doc->endScriptUndo();
		return;
	}
	const size_t pos = text.find(_scriptFind);
	if (pos == std::string::npos)
		return;
	// InputTextMultiline does not expose caret; copy the match to the status via tooltip.
	ImGui::SetClipboardText(text.substr(pos, std::strlen(_scriptFind)).c_str());
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

void UIMapEditorWindow::renderItemBounds (ImDrawList* drawList, const MapEditorTileItem& item, float originX,
		float originY, float tileW, float tileH, ImU32 lineCol, ImU32 fillCol, float thickness) const
{
	if (!item.def)
		return;
	const vec2 size = item.getSize(false);
	const float hx = originX + (item.gridX + item.getX(false)) * tileW - _panX;
	const float hy = originY + (item.gridY + item.getY(false)) * tileH - _panY;
	const ImVec2 a(hx, hy);
	const ImVec2 b(hx + size.x * tileW, hy + size.y * tileH);
	if (fillCol != 0)
		drawList->AddRectFilled(a, b, fillCol);
	drawList->AddRect(a, b, lineCol, 0.0f, 0, thickness);
}

IMapEditorDocument::MapEdge UIMapEditorWindow::hitTestMapEdge (float tileW, float tileH) const
{
	using MapEdge = IMapEditorDocument::MapEdge;
	const ImVec2 mouse = ImGui::GetIO().MousePos;
	const float mapLeft = _canvasMinX - _panX;
	const float mapTop = _canvasMinY - _panY;
	const float mapRight = mapLeft + static_cast<float>(_doc->getMapWidth()) * tileW;
	const float mapBottom = mapTop + static_cast<float>(_doc->getMapHeight()) * tileH;
	const float outside = 12.0f;
	const float inside = std::min(6.0f, std::max(2.0f, tileW * 0.2f));
	const bool alongV = mouse.y >= mapTop - outside && mouse.y <= mapBottom + outside;
	const bool alongH = mouse.x >= mapLeft - outside && mouse.x <= mapRight + outside;

	float best = outside + 1.0f;
	MapEdge edge = MapEdge::None;
	auto consider = [&] (MapEdge candidate, float dist, bool along) {
		if (!along || dist > best)
			return;
		best = dist;
		edge = candidate;
	};
	if (mouse.x >= mapLeft - outside && mouse.x <= mapLeft + inside)
		consider(MapEdge::Left, std::fabs(mouse.x - mapLeft), alongV);
	if (mouse.x >= mapRight - inside && mouse.x <= mapRight + outside)
		consider(MapEdge::Right, std::fabs(mouse.x - mapRight), alongV);
	if (mouse.y >= mapTop - outside && mouse.y <= mapTop + inside)
		consider(MapEdge::Top, std::fabs(mouse.y - mapTop), alongH);
	if (mouse.y >= mapBottom - inside && mouse.y <= mapBottom + outside)
		consider(MapEdge::Bottom, std::fabs(mouse.y - mapBottom), alongH);
	return edge;
}

bool UIMapEditorWindow::handleMapEdgeResize (float tileW, float tileH, bool allowHover) const
{
	using MapEdge = IMapEditorDocument::MapEdge;
	const bool space = ImGui::IsKeyDown(ImGuiKey_Space);

	if (_mapEdgeDragging == MapEdge::None) {
		_mapEdgeHover = MapEdge::None;
		if (allowHover && !space && !_panning)
			_mapEdgeHover = hitTestMapEdge(tileW, tileH);
	}

	const MapEdge edge = _mapEdgeDragging != MapEdge::None ? _mapEdgeDragging : _mapEdgeHover;
	if (edge != MapEdge::None) {
		if (edge == MapEdge::Left || edge == MapEdge::Right)
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		else
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
		if (_mapEdgeDragging == MapEdge::None)
			ImGui::SetTooltip("%s", tr("Drag to resize this side").c_str());
	}

	if (_mapEdgeDragging != MapEdge::None) {
		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			_mapEdgeDragging = MapEdge::None;
			return true;
		}
		const ImVec2 mouse = ImGui::GetIO().MousePos;
		int target = 0;
		switch (_mapEdgeDragging) {
		case MapEdge::Right:
			target = static_cast<int>(std::lround((mouse.x - _mapResizeStartMouseX) / tileW));
			break;
		case MapEdge::Left:
			target = static_cast<int>(std::lround((_mapResizeStartMouseX - mouse.x) / tileW));
			break;
		case MapEdge::Bottom:
			target = static_cast<int>(std::lround((mouse.y - _mapResizeStartMouseY) / tileH));
			break;
		case MapEdge::Top:
			target = static_cast<int>(std::lround((_mapResizeStartMouseY - mouse.y) / tileH));
			break;
		default:
			break;
		}
		const int need = target - _mapResizeApplied;
		if (need != 0) {
			const int applied = _doc->resizeFromEdge(_mapEdgeDragging, need, false);
			if (applied != 0) {
				_mapResizeApplied += applied;
				if (_mapEdgeDragging == MapEdge::Left)
					_panX += static_cast<float>(applied) * tileW;
				else if (_mapEdgeDragging == MapEdge::Top)
					_panY += static_cast<float>(applied) * tileH;
			}
		}
		return true;
	}

	if (_mapEdgeHover != MapEdge::None && allowHover && !space && !_panning) {
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			_mapEdgeDragging = _mapEdgeHover;
			_mapResizeStartMouseX = ImGui::GetIO().MousePos.x;
			_mapResizeStartMouseY = ImGui::GetIO().MousePos.y;
			_mapResizeApplied = 0;
			_doc->beginUndoStroke();
		}
		return true;
	}
	return false;
}

void UIMapEditorWindow::renderMapResizeHandles (ImDrawList* drawList, float originX, float originY, float tileW,
		float tileH) const
{
	using MapEdge = IMapEditorDocument::MapEdge;
	const MapEdge edge = _mapEdgeDragging != MapEdge::None ? _mapEdgeDragging : _mapEdgeHover;
	if (edge == MapEdge::None || drawList == nullptr)
		return;

	const float mapLeft = originX - _panX;
	const float mapTop = originY - _panY;
	const float mapRight = mapLeft + static_cast<float>(_doc->getMapWidth()) * tileW;
	const float mapBottom = mapTop + static_cast<float>(_doc->getMapHeight()) * tileH;
	const float midX = (mapLeft + mapRight) * 0.5f;
	const float midY = (mapTop + mapBottom) * 0.5f;
	const ImU32 outline = IM_COL32(255, 176, 64, 90);
	const ImU32 active = IM_COL32(255, 176, 64, 240);
	const ImU32 grip = IM_COL32(255, 176, 64, 210);
	drawList->AddRect(ImVec2(mapLeft, mapTop), ImVec2(mapRight, mapBottom), outline, 0.0f, 0, 1.0f);

	const float gripHalf = 22.0f;
	const float gripThick = 8.0f;
	switch (edge) {
	case MapEdge::Left:
		drawList->AddLine(ImVec2(mapLeft, mapTop), ImVec2(mapLeft, mapBottom), active, 3.0f);
		drawList->AddRectFilled(ImVec2(mapLeft - 2.0f, midY - gripHalf), ImVec2(mapLeft + gripThick, midY + gripHalf),
				grip, 3.0f);
		drawList->AddTriangleFilled(ImVec2(mapLeft + 2.0f, midY), ImVec2(mapLeft + 12.0f, midY - 7.0f),
				ImVec2(mapLeft + 12.0f, midY + 7.0f), active);
		break;
	case MapEdge::Right:
		drawList->AddLine(ImVec2(mapRight, mapTop), ImVec2(mapRight, mapBottom), active, 3.0f);
		drawList->AddRectFilled(ImVec2(mapRight - gripThick, midY - gripHalf), ImVec2(mapRight + 2.0f, midY + gripHalf),
				grip, 3.0f);
		drawList->AddTriangleFilled(ImVec2(mapRight - 2.0f, midY), ImVec2(mapRight - 12.0f, midY - 7.0f),
				ImVec2(mapRight - 12.0f, midY + 7.0f), active);
		break;
	case MapEdge::Top:
		drawList->AddLine(ImVec2(mapLeft, mapTop), ImVec2(mapRight, mapTop), active, 3.0f);
		drawList->AddRectFilled(ImVec2(midX - gripHalf, mapTop - 2.0f), ImVec2(midX + gripHalf, mapTop + gripThick),
				grip, 3.0f);
		drawList->AddTriangleFilled(ImVec2(midX, mapTop + 2.0f), ImVec2(midX - 7.0f, mapTop + 12.0f),
				ImVec2(midX + 7.0f, mapTop + 12.0f), active);
		break;
	case MapEdge::Bottom:
		drawList->AddLine(ImVec2(mapLeft, mapBottom), ImVec2(mapRight, mapBottom), active, 3.0f);
		drawList->AddRectFilled(ImVec2(midX - gripHalf, mapBottom - gripThick), ImVec2(midX + gripHalf, mapBottom + 2.0f),
				grip, 3.0f);
		drawList->AddTriangleFilled(ImVec2(midX, mapBottom - 2.0f), ImVec2(midX - 7.0f, mapBottom - 12.0f),
				ImVec2(midX + 7.0f, mapBottom - 12.0f), active);
		break;
	default:
		break;
	}

	char buf[32];
	std::snprintf(buf, sizeof(buf), "%i × %i", _doc->getMapWidth(), _doc->getMapHeight());
	ImVec2 textPos(midX + 10.0f, midY - 8.0f);
	switch (edge) {
	case MapEdge::Left:
		textPos = ImVec2(mapLeft + 10.0f, midY - 8.0f);
		break;
	case MapEdge::Right:
		textPos = ImVec2(mapRight - 70.0f, midY - 8.0f);
		break;
	case MapEdge::Top:
		textPos = ImVec2(midX - 28.0f, mapTop + 8.0f);
		break;
	case MapEdge::Bottom:
		textPos = ImVec2(midX - 28.0f, mapBottom - 22.0f);
		break;
	default:
		break;
	}
	drawList->AddText(textPos, IM_COL32(255, 230, 180, 255), buf);
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
		if (item.entityType != nullptr && (item.amount > 1 || item.delay > 0)) {
			const float tx = x + item.gridX * tileW - _panX + 2.0f;
			const float ty = y + item.gridY * tileH - _panY + 2.0f;
			char buf[32];
			std::snprintf(buf, sizeof(buf), "%ix/%ims", item.amount, item.delay);
			drawList->AddText(ImVec2(tx, ty), IM_COL32(255, 230, 120, 255), buf);
		}
	}

	if (_doc->hasRegion()) {
		int rx0, ry0, rx1, ry1;
		_doc->getRegion(rx0, ry0, rx1, ry1);
		const float x0 = x + rx0 * tileW - _panX;
		const float y0 = y + ry0 * tileH - _panY;
		const float x1 = x + (rx1 + 1) * tileW - _panX;
		const float y1 = y + (ry1 + 1) * tileH - _panY;
		drawList->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(80, 200, 255, 220), 0.0f, 0, 2.0f);
		drawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(80, 180, 255, 40));
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

	const bool resizingMap = _mapEdgeHover != IMapEditorDocument::MapEdge::None
			|| _mapEdgeDragging != IMapEditorDocument::MapEdge::None;
	if (!resizingMap && _canvasHovered && _doc->getTool() == IMapEditorDocument::Tool::Pick) {
		if (const MapEditorTileItem* hover = _doc->getTileAtCursor(ImGui::GetIO().KeyAlt)) {
			renderItemBounds(drawList, *hover, x, y, tileW, tileH, IM_COL32(80, 220, 255, 255),
					IM_COL32(80, 220, 255, 50), 2.0f);
			if (hover->def) {
				const float lx = x + (hover->gridX + hover->getX(false)) * tileW - _panX;
				const float ly = y + (hover->gridY + hover->getY(false)) * tileH - _panY;
				drawList->AddText(ImVec2(lx, ly - 16.0f), IM_COL32(160, 230, 255, 255), hover->def->id.c_str());
			}
		}
	}
	if (const MapEditorTileItem* selected = _doc->getHighlightItem()) {
		renderItemBounds(drawList, *selected, x, y, tileW, tileH, IM_COL32(255, 255, 0, 255), 0, 2.0f);
	}

	if (!resizingMap && _canvasHovered && _doc->getActiveSprite() && _doc->getTool() != IMapEditorDocument::Tool::Pick) {
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
		char brushBuf[48];
		if (_doc->getActiveEntityType() != nullptr)
			std::snprintf(brushBuf, sizeof(brushBuf), "%s %s", _doc->isActiveEntityRight() ? "R" : "L",
					_doc->getActiveEntityType()->name.c_str());
		else
			std::snprintf(brushBuf, sizeof(brushBuf), "%i°", _doc->getActiveAngle());
		drawList->AddText(ImVec2(hx, hy - 16.0f), IM_COL32(180, 255, 180, 255), brushBuf);
	}

	renderMapResizeHandles(drawList, x, y, tileW, tileH);

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
	const ImVec2 mouse = ImGui::GetIO().MousePos;
	if (_canvasHovered || _movingItem) {
		const float gx = (mouse.x - _canvasMinX + _panX) / tileW;
		const float gy = (mouse.y - _canvasMinY + _panY) / tileH;
		_doc->setCursorGrid(gx, gy);
		if (_canvasHovered)
			_doc->setSelectedGrid(std::floor(gx), std::floor(gy));
	}

	const bool overlayConsumed = handleCanvasOverlayInput(tileW, tileH);
	const bool space = ImGui::IsKeyDown(ImGuiKey_Space);
	if (_canvasHovered && !overlayConsumed && _mapEdgeDragging == IMapEditorDocument::MapEdge::None) {
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || (space && ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
			_panning = true;
			_panX -= ImGui::GetIO().MouseDelta.x;
			_panY -= ImGui::GetIO().MouseDelta.y;
		} else {
			_panning = false;
		}
		if (_panning || space)
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

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
	}

	const bool edgeConsumed = handleMapEdgeResize(tileW, tileH,
			_canvasHovered && !overlayConsumed && !ImGui::GetIO().KeyAlt && !ImGui::GetIO().KeyShift);
	if (_canvasHovered && !overlayConsumed && !edgeConsumed) {
		if (!_panning && !space && _doc->getTool() == IMapEditorDocument::Tool::Pick) {
			if (const MapEditorTileItem* hover = _doc->getTileAtCursor(ImGui::GetIO().KeyAlt)) {
				if (hover->allowsSubTileX())
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}
		}

		if (!_panning && !space) {
			const bool shift = ImGui::GetIO().KeyShift;
			const bool alt = ImGui::GetIO().KeyAlt;
			const int gx = static_cast<int>(std::floor(_doc->getSelectedGridX()));
			const int gy = static_cast<int>(std::floor(_doc->getSelectedGridY()));
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				_movingItem = false;
				_moveUndoStarted = false;
				if (alt) {
					_doc->pickTopmostAtSelection();
				} else if (shift || _doc->getTool() == IMapEditorDocument::Tool::Pick) {
					if (shift) {
						_regionDragging = true;
						_regionAnchorX = gx;
						_regionAnchorY = gy;
						_doc->setRegion(gx, gy, gx, gy);
					} else {
						_doc->pickAtSelection();
						_doc->clearRegion();
						if (const MapEditorTileItem* hit = _doc->getHighlightItem()) {
							if (hit->allowsSubTileX()) {
								_movingItem = true;
								_moveGrabOffsetX = _doc->getCursorGridX() - hit->gridX;
							}
						}
					}
				} else {
					_doc->beginUndoStroke();
					_doc->paintAtSelection(true, false);
				}
			} else if (_regionDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				_doc->setRegion(_regionAnchorX, _regionAnchorY, gx, gy);
			} else if (!shift && !alt && !_movingItem && _doc->getTool() != IMapEditorDocument::Tool::Fill
					&& _doc->getTool() != IMapEditorDocument::Tool::Pick
					&& ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				_doc->paintAtSelection(true, false);
			}
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				_regionDragging = false;
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				_doc->beginUndoStroke();
				_doc->eraseAtSelection(false);
			} else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
				_doc->eraseAtSelection(false);
			}
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && !ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
				if (alt)
					_doc->pickTopmostAtSelection();
				else
					_doc->pickAtSelection();
			}
		}
	}

	if (_movingItem && _mapEdgeDragging == IMapEditorDocument::MapEdge::None
			&& ImGui::IsMouseDragging(ImGuiMouseButton_Left) && _doc->getHighlightItem() != nullptr) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		if (!_moveUndoStarted) {
			_doc->beginUndoStroke();
			_moveUndoStarted = true;
		}
		const gridCoord x = std::round((_doc->getCursorGridX() - _moveGrabOffsetX) * 10.0f) / 10.0f;
		_doc->setHighlightPosition(x, _doc->getHighlightItem()->gridY);
	}

	// Commit paint/erase strokes even if the cursor left the canvas before release.
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
		_doc->endUndoStroke();
		_movingItem = false;
		_moveUndoStarted = false;
		_mapEdgeDragging = IMapEditorDocument::MapEdge::None;
	}

	renderMapIntoCanvas(ImGui::GetWindowDrawList());
	ImGui::EndChild();
}

void UIMapEditorWindow::setupEditorDockSpace () const
{
	const ImGuiID dockspaceId = ImGui::GetID("MapEditorDockSpace_v2");
	if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetContentRegionAvail());
		ImGuiID dockMain = dockspaceId;
		const ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.22f, nullptr, &dockMain);
		const ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.28f, nullptr, &dockMain);
		ImGui::DockBuilderDockWindow("Palette###editor_palette", dockLeft);
		ImGui::DockBuilderDockWindow("Map###editor_map", dockMain);
		ImGui::DockBuilderDockWindow("Layers###editor_layers", dockRight);
		ImGui::DockBuilderDockWindow("Shapes###editor_shapes", dockRight);
		if (_doc->supportsMapScript())
			ImGui::DockBuilderDockWindow("Script###editor_script", dockRight);
		ImGui::DockBuilderDockWindow("Properties###editor_properties", dockRight);
		ImGui::DockBuilderFinish(dockspaceId);
	}
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void UIMapEditorWindow::render (int x, int y) const
{
	const float pw = static_cast<float>(getRenderWidth(false));
	const float ph = static_cast<float>(getRenderHeight(false));
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(getRenderX(false)), static_cast<float>(getRenderY(false))), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(pw, ph), ImGuiCond_Always);
	const ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking;
	ImGui::Begin(tr("Map Editor").c_str(), nullptr, hostFlags);

	handleHotkeys();
	drawToolbar();
	ImGui::Separator();
	setupEditorDockSpace();
	ImGui::End();

	if (ImGui::Begin((tr("Palette") + "###editor_palette").c_str())) {
		if (ImGui::BeginTabBar("left_tabs")) {
			if (ImGui::BeginTabItem(tr("Tiles").c_str())) {
				_doc->setEditMode(IMapEditorDocument::EditMode::Tiles);
				drawTilesPanel();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(tr("Entities").c_str())) {
				_doc->setEditMode(IMapEditorDocument::EditMode::Entities);
				drawEntitiesPanel();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(tr("Maps").c_str())) {
				drawMapsPanel();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin((tr("Map") + "###editor_map").c_str(), nullptr, ImGuiWindowFlags_NoScrollbar))
		drawCanvas();
	ImGui::End();
	ImGui::PopStyleVar();

	if (ImGui::Begin((tr("Properties") + "###editor_properties").c_str()))
		drawPropertiesPanel();
	ImGui::End();

	if (ImGui::Begin((tr("Layers") + "###editor_layers").c_str()))
		drawLayersPanel();
	ImGui::End();

	if (_showHelp) {
		if (ImGui::Begin((tr("Help") + "###editor_help").c_str(), &_showHelp))
			drawHelpPanel();
		ImGui::End();
	}

	drawConfirmModal();
	drawValidationModal();
	drawScriptEditor();
	drawDefinitionEditor();
	{
		std::string suggested;
		if (_doc->getHighlightItem() && _doc->getHighlightItem()->def)
			suggested = _doc->getHighlightItem()->def->id;
		else if (_doc->getActiveSprite())
			suggested = _doc->getActiveSprite()->id;
		_shapeEditor.draw(_frontendPtr, suggested);
	}
}
