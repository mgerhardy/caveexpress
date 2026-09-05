#include "ui/editor/UIShapeEditor.h"
#include "ui/editor/ImGuiTextureDraw.h"
#include "common/SpritePolygonLua.h"
#include "common/SpriteLuaPatcher.h"
#include "common/String.h"
#include "common/FileSystem.h"
#include "common/File.h"
#include "common/LUALibrary.h"
#include "sprites/Sprite.h"
#include "ui/UI.h"
#include "imgui_stdlib.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
const float ZOOM_MIN = 0.25f;
const float ZOOM_MAX = 32.0f;
const float VERTEX_HIT_RADIUS = 8.0f;
const float DRAG_THRESHOLD = 3.0f;
const int MAX_PHYSICS_POLYGON_VERTICES = 8;

bool passesFilter (const std::string& text, const char* filter)
{
	if (filter == nullptr || filter[0] == '\0')
		return true;
	return string::toLower(text).find(string::toLower(filter)) != std::string::npos;
}

float shoelaceSigned (const SpritePolygon::SpriteVector& verts)
{
	if (verts.size() < 3)
		return 0.0f;
	float area = 0.0f;
	for (size_t i = 0; i < verts.size(); ++i) {
		const SpriteVertex& a = verts[i];
		const SpriteVertex& b = verts[(i + 1) % verts.size()];
		area += a.x * b.y - b.x * a.y;
	}
	return area * 0.5f;
}

bool isConvex (const SpritePolygon::SpriteVector& verts)
{
	if (verts.size() < 3)
		return true;
	int sign = 0;
	const size_t n = verts.size();
	for (size_t i = 0; i < n; ++i) {
		const SpriteVertex& a = verts[i];
		const SpriteVertex& b = verts[(i + 1) % n];
		const SpriteVertex& c = verts[(i + 2) % n];
		const float cross = (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
		if (std::fabs(cross) < 1.0e-6f)
			continue;
		const int s = cross > 0.0f ? 1 : -1;
		if (sign == 0)
			sign = s;
		else if (s != sign)
			return false;
	}
	return true;
}
}

void UIShapeEditor::open (const std::string& suggestedSpriteId)
{
	_visible = true;
	if (!suggestedSpriteId.empty() && suggestedSpriteId != _spriteId)
		loadSprite(suggestedSpriteId);
}

SpriteDefPtr UIShapeEditor::currentDef () const
{
	if (_spriteId.empty())
		return SpriteDefPtr();
	return SpriteDefinition::get().getSpriteDefinition(_spriteId);
}

void UIShapeEditor::loadSprite (const std::string& id)
{
	if (id.empty())
		return;
	const SpriteDefPtr def = SpriteDefinition::get().getSpriteDefinition(id);
	if (!def)
		return;
	_spriteId = id;
	_polygons = def->polygons;
	_circles = def->circles;
	_activePoly = 0;
	_activeCircle = _circles.empty() ? -1 : 0;
	if (_polygons.empty())
		_polygons.push_back(SpritePolygon(""));
	if (_activePoly >= static_cast<int>(_polygons.size()))
		_activePoly = static_cast<int>(_polygons.size()) - 1;
	std::snprintf(_userDataBuf, sizeof(_userDataBuf), "%s", _polygons[_activePoly].userData.c_str());
	_luaDirty = true;
	_luaEdited = false;
	_dragPoly = _dragVert = _dragCircle = -1;
	_dragging = false;
	_hasShapeUndo = false;
	_zoom = 4.0f;
	_panX = _panY = 0.0f;
}

void UIShapeEditor::applyToDef ()
{
	const SpriteDefPtr def = currentDef();
	if (!def)
		return;
	def->polygons = _polygons;
	def->circles = _circles;
	def->invalidateShapeSize();
}

void UIShapeEditor::rebuildLua ()
{
	_luaText = sprite_polygon_lua::toLuaShapes(_polygons, _circles);
	_luaDirty = false;
}

void UIShapeEditor::pushShapeUndo ()
{
	_undoPolygons = _polygons;
	_undoCircles = _circles;
	_hasShapeUndo = true;
}

void UIShapeEditor::setStatus (const std::string& text)
{
	_status = text;
	_statusUntil = ImGui::GetTime() + 2.5;
}

bool UIShapeEditor::writeSpritesLua ()
{
	applyToDef();
	const std::string path = FS.getDataDir() + "sprites.lua";
	SDL_RWops* rwops = FS.createRWops(path, "rb");
	FilePtr file = rwops != nullptr ? FilePtr(new File(rwops, path)) : FS.getFile("sprites.lua");
	if (!file || !file->exists()) {
		setStatus(tr("Could not open sprites.lua"));
		return false;
	}
	void* buf = nullptr;
	const int n = file->read(&buf);
	if (n <= 0 || buf == nullptr) {
		delete[] static_cast<char*>(buf);
		setStatus(tr("Could not read sprites.lua"));
		return false;
	}
	const std::string source(static_cast<char*>(buf), static_cast<size_t>(n));
	delete[] static_cast<char*>(buf);
	std::string patched;
	std::string error;
	if (!sprite_lua_patcher::patchSpriteShapes(source, _spriteId, _polygons, _circles, patched, &error)) {
		setStatus(error.empty() ? tr("Failed to patch sprites.lua") : error);
		return false;
	}
	LUA check;
	if (!check.loadBuffer(patched, "sprites.lua")) {
		setStatus(tr("Patched sprites.lua is not valid Lua"));
		return false;
	}
	if (FS.writeSysFile(path, reinterpret_cast<const unsigned char*>(patched.c_str()), patched.size(), true) < 0) {
		setStatus(tr("Failed to write sprites.lua"));
		return false;
	}
	setStatus(tr("Wrote sprites.lua"));
	return true;
}

int UIShapeEditor::hitCircle (const ImVec2& origin, float tilePixels, const ImVec2& mouse, bool& onRim) const
{
	onRim = false;
	int best = -1;
	float bestDist = VERTEX_HIT_RADIUS * 2.0f;
	for (int c = 0; c < static_cast<int>(_circles.size()); ++c) {
		const ImVec2 center = toScreen(origin, tilePixels, _circles[c].center.x, _circles[c].center.y);
		const float dx = mouse.x - center.x;
		const float dy = mouse.y - center.y;
		const float dist = std::sqrt(dx * dx + dy * dy);
		const float radiusPx = _circles[c].radius * tilePixels;
		if (dist <= VERTEX_HIT_RADIUS && dist < bestDist) {
			best = c;
			bestDist = dist;
			onRim = false;
		} else if (std::fabs(dist - radiusPx) <= VERTEX_HIT_RADIUS && std::fabs(dist - radiusPx) < bestDist) {
			best = c;
			bestDist = std::fabs(dist - radiusPx);
			onRim = true;
		}
	}
	return best;
}

void UIShapeEditor::ensureActivePolygon ()
{
	if (_polygons.empty()) {
		_polygons.push_back(SpritePolygon(""));
		_activePoly = 0;
		_userDataBuf[0] = '\0';
		_luaDirty = true;
	}
	if (_activePoly < 0 || _activePoly >= static_cast<int>(_polygons.size()))
		_activePoly = static_cast<int>(_polygons.size()) - 1;
}

ImVec2 UIShapeEditor::toScreen (const ImVec2& origin, float tilePixels, float vx, float vy) const
{
	return ImVec2(origin.x + vx * tilePixels, origin.y - vy * tilePixels);
}

void UIShapeEditor::toVertex (const ImVec2& origin, float tilePixels, const ImVec2& screen, float& vx, float& vy) const
{
	if (tilePixels <= 0.0f) {
		vx = vy = 0.0f;
		return;
	}
	vx = (screen.x - origin.x) / tilePixels;
	vy = (origin.y - screen.y) / tilePixels;
}

bool UIShapeEditor::hitVertex (const ImVec2& origin, float tilePixels, const ImVec2& mouse, int& outPoly, int& outVert) const
{
	outPoly = -1;
	outVert = -1;
	float best = VERTEX_HIT_RADIUS * VERTEX_HIT_RADIUS;
	for (int p = 0; p < static_cast<int>(_polygons.size()); ++p) {
		const SpritePolygon::SpriteVector& verts = _polygons[p].vertices;
		for (int v = 0; v < static_cast<int>(verts.size()); ++v) {
			const ImVec2 s = toScreen(origin, tilePixels, verts[v].x, verts[v].y);
			const float dx = s.x - mouse.x;
			const float dy = s.y - mouse.y;
			const float d2 = dx * dx + dy * dy;
			if (d2 <= best) {
				best = d2;
				outPoly = p;
				outVert = v;
			}
		}
	}
	return outPoly >= 0;
}

void UIShapeEditor::drawSpritePicker ()
{
	ImGui::SetNextItemWidth(220.0f);
	ImGui::InputText(tr("Filter").c_str(), _filter, sizeof(_filter));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-1.0f);
	const char* preview = _spriteId.empty() ? tr("Select a sprite").c_str() : _spriteId.c_str();
	if (ImGui::BeginCombo("##shape_sprite", preview)) {
		for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
			if (!passesFilter(i->first, _filter))
				continue;
			const bool selected = i->first == _spriteId;
			if (ImGui::Selectable(i->first.c_str(), selected))
				loadSprite(i->first);
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void UIShapeEditor::drawToolbar ()
{
	ensureActivePolygon();
	if (ImGui::Button(tr("New polygon").c_str())) {
		_polygons.push_back(SpritePolygon(""));
		_activePoly = static_cast<int>(_polygons.size()) - 1;
		_userDataBuf[0] = '\0';
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Delete polygon").c_str()) && !_polygons.empty()) {
		_polygons.erase(_polygons.begin() + _activePoly);
		if (_polygons.empty())
			_polygons.push_back(SpritePolygon(""));
		if (_activePoly >= static_cast<int>(_polygons.size()))
			_activePoly = static_cast<int>(_polygons.size()) - 1;
		std::snprintf(_userDataBuf, sizeof(_userDataBuf), "%s", _polygons[_activePoly].userData.c_str());
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Delete vertex").c_str()) && _activePoly >= 0
			&& _activePoly < static_cast<int>(_polygons.size()) && !_polygons[_activePoly].vertices.empty()) {
		_polygons[_activePoly].vertices.pop_back();
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Clear").c_str()) && _activePoly >= 0 && _activePoly < static_cast<int>(_polygons.size())) {
		_polygons[_activePoly].vertices.clear();
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("New circle").c_str())) {
		pushShapeUndo();
		SpriteCircle circle("");
		circle.radius = 0.25f;
		_circles.push_back(circle);
		_activeCircle = static_cast<int>(_circles.size()) - 1;
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Delete circle").c_str()) && _activeCircle >= 0
			&& _activeCircle < static_cast<int>(_circles.size())) {
		pushShapeUndo();
		_circles.erase(_circles.begin() + _activeCircle);
		_activeCircle = _circles.empty() ? -1 : std::min(_activeCircle, static_cast<int>(_circles.size()) - 1);
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Undo shape").c_str()) && _hasShapeUndo) {
		_polygons = _undoPolygons;
		_circles = _undoCircles;
		_hasShapeUndo = false;
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Write sprites.lua").c_str()))
		writeSpritesLua();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tr("Patch this sprite's polygons/circles in the game data sprites.lua").c_str());
	ImGui::SameLine();
	if (ImGui::Button(tr("Fit").c_str())) {
		_zoom = 4.0f;
		_panX = _panY = 0.0f;
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80.0f);
	int polyIndex = _activePoly + 1;
	const int polyCount = static_cast<int>(_polygons.size());
	if (ImGui::InputInt(tr("Polygon").c_str(), &polyIndex)) {
		_activePoly = std::max(1, std::min(polyCount, polyIndex)) - 1;
		std::snprintf(_userDataBuf, sizeof(_userDataBuf), "%s", _polygons[_activePoly].userData.c_str());
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.0f);
	if (ImGui::InputText(tr("User data").c_str(), _userDataBuf, sizeof(_userDataBuf))) {
		_polygons[_activePoly].userData = _userDataBuf;
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}

	if (_activePoly >= 0 && _activePoly < static_cast<int>(_polygons.size())) {
		const SpritePolygon::SpriteVector& verts = _polygons[_activePoly].vertices;
		const int n = static_cast<int>(verts.size());
		if (n > MAX_PHYSICS_POLYGON_VERTICES) {
			ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.2f, 1.0f), "%s (%i > %i)",
					tr("Too many vertices for Box2D").c_str(), n, MAX_PHYSICS_POLYGON_VERTICES);
		} else if (n >= 3 && !isConvex(verts)) {
			ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "%s",
					tr("Polygon is concave — Box2D needs convex shapes (split it).").c_str());
		} else {
			ImGui::TextDisabled("%s: %i  |  %s", tr("Vertices").c_str(), n,
					tr("LMB place / drag, RMB delete vertex, wheel zoom, MMB pan").c_str());
		}
	}
}

void UIShapeEditor::drawCanvas (IFrontend* frontend)
{
	const ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
	if (canvasSize.x < 32.0f || canvasSize.y < 32.0f)
		return;

	ImGui::InvisibleButton("shape_canvas", canvasSize,
			ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
	const bool hovered = ImGui::IsItemHovered();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);
	drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
			IM_COL32(28, 28, 32, 255));

	const TexturePtr tileRef = UI::get().loadTexture("tile-reference");
	const float tileRefW = tileRef && tileRef->getWidth() > 0 ? static_cast<float>(tileRef->getWidth()) : 16.0f;
	const float tilePixels = tileRefW * _zoom;
	const ImVec2 origin(canvasPos.x + canvasSize.x * 0.5f - _panX, canvasPos.y + canvasSize.y * 0.5f - _panY);

	const SpriteDefPtr def = currentDef();
	const float spriteW = def ? def->width : 1.0f;
	const float spriteH = def ? def->height : 1.0f;

	if (def) {
		const SpritePtr sprite = UI::get().loadSprite(def->id);
		int texW = sprite ? sprite->getMaxWidth() : 0;
		int texH = sprite ? sprite->getMaxHeight() : 0;
		if (texW <= 0 || texH <= 0) {
			texW = static_cast<int>(spriteW * tileRefW);
			texH = static_cast<int>(spriteH * tileRefW);
		}
		const float imgW = static_cast<float>(texW) * _zoom;
		const float imgH = static_cast<float>(texH) * _zoom;
		const ImVec2 imgMin(origin.x - imgW * 0.5f, origin.y - imgH * 0.5f);
		const ImVec2 imgMax(origin.x + imgW * 0.5f, origin.y + imgH * 0.5f);
		if (sprite)
			mapEditorAddSprite(drawList, frontend, sprite, imgMin, imgMax, 1.0f, 0);
		else
			drawList->AddRectFilled(imgMin, imgMax, IM_COL32(60, 60, 70, 255));
	}

	const ImVec2 tileMin = toScreen(origin, tilePixels, -spriteW * 0.5f, spriteH * 0.5f);
	const ImVec2 tileMax = toScreen(origin, tilePixels, spriteW * 0.5f, -spriteH * 0.5f);
	drawList->AddRect(tileMin, tileMax, IM_COL32(80, 160, 255, 140), 0.0f, 0, 1.0f);
	drawList->AddLine(ImVec2(tileMin.x, origin.y), ImVec2(tileMax.x, origin.y), IM_COL32(80, 160, 255, 70));
	drawList->AddLine(ImVec2(origin.x, tileMin.y), ImVec2(origin.x, tileMax.y), IM_COL32(80, 160, 255, 70));

	for (int c = 0; c < static_cast<int>(_circles.size()); ++c) {
		const SpriteCircle& circle = _circles[c];
		const ImVec2 center = toScreen(origin, tilePixels, circle.center.x, circle.center.y);
		const ImU32 col = c == _activeCircle ? IM_COL32(80, 255, 255, 240) : IM_COL32(80, 220, 220, 180);
		drawList->AddCircle(center, std::max(2.0f, circle.radius * tilePixels), col, 32, c == _activeCircle ? 2.0f : 1.5f);
		drawList->AddCircleFilled(center, 4.0f, IM_COL32(40, 200, 200, 255));
	}

	const ImVec2 mouse = ImGui::GetIO().MousePos;
	_hoverPoly = _hoverVert = -1;
	if (hovered)
		hitVertex(origin, tilePixels, mouse, _hoverPoly, _hoverVert);

	const bool space = ImGui::IsKeyDown(ImGuiKey_Space);
	if (hovered) {
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || (space && ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
			_panX -= ImGui::GetIO().MouseDelta.x;
			_panY -= ImGui::GetIO().MouseDelta.y;
		}
		const float wheel = ImGui::GetIO().MouseWheel;
		if (wheel != 0.0f) {
			const float oldZoom = _zoom;
			_zoom = std::max(ZOOM_MIN, std::min(ZOOM_MAX, _zoom + wheel * 0.15f * _zoom));
			const float ratio = _zoom / oldZoom;
			const float mx = mouse.x - (canvasPos.x + canvasSize.x * 0.5f);
			const float my = mouse.y - (canvasPos.y + canvasSize.y * 0.5f);
			_panX = (_panX + mx) * ratio - mx;
			_panY = (_panY + my) * ratio - my;
		}

		if (!space && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			_pressMouse = mouse;
			bool onRim = false;
			const int hitC = hitCircle(origin, tilePixels, mouse, onRim);
			_pressOnVertex = _hoverPoly >= 0;
			if (hitC >= 0 && !_pressOnVertex) {
				pushShapeUndo();
				_dragCircle = hitC;
				_dragCircleRadius = onRim;
				_activeCircle = hitC;
			} else if (_pressOnVertex) {
				pushShapeUndo();
				_dragPoly = _hoverPoly;
				_dragVert = _hoverVert;
				_activePoly = _hoverPoly;
				std::snprintf(_userDataBuf, sizeof(_userDataBuf), "%s", _polygons[_activePoly].userData.c_str());
			}
			_dragging = false;
		}
		if (!space && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && _hoverPoly >= 0 && _hoverVert >= 0) {
			SpritePolygon::SpriteVector& verts = _polygons[_hoverPoly].vertices;
			verts.erase(verts.begin() + _hoverVert);
			_activePoly = _hoverPoly;
			_luaDirty = true;
			_luaEdited = false;
			applyToDef();
		}
	}

	if (_dragCircle >= 0 && _dragCircle < static_cast<int>(_circles.size()) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		float vx, vy;
		toVertex(origin, tilePixels, mouse, vx, vy);
		if (_dragCircleRadius) {
			const float dx = vx - _circles[_dragCircle].center.x;
			const float dy = vy - _circles[_dragCircle].center.y;
			_circles[_dragCircle].radius = std::max(0.02f, std::sqrt(dx * dx + dy * dy));
		} else {
			_circles[_dragCircle].center.x = vx;
			_circles[_dragCircle].center.y = vy;
		}
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	} else if (_dragPoly >= 0 && _dragVert >= 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		const float dx = mouse.x - _pressMouse.x;
		const float dy = mouse.y - _pressMouse.y;
		if (_dragging || dx * dx + dy * dy >= DRAG_THRESHOLD * DRAG_THRESHOLD) {
			_dragging = true;
			float vx, vy;
			toVertex(origin, tilePixels, mouse, vx, vy);
			if (ImGui::GetIO().KeyCtrl) {
				vx = std::round(vx * 100.0f) / 100.0f;
				vy = std::round(vy * 100.0f) / 100.0f;
			}
			_polygons[_dragPoly].vertices[_dragVert].x = vx;
			_polygons[_dragPoly].vertices[_dragVert].y = vy;
			_luaDirty = true;
			_luaEdited = false;
			applyToDef();
		}
	}
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		if (!_dragging && !_pressOnVertex && _dragCircle < 0 && hovered && !space) {
			pushShapeUndo();
			ensureActivePolygon();
			float vx, vy;
			toVertex(origin, tilePixels, mouse, vx, vy);
			if (ImGui::GetIO().KeyCtrl) {
				vx = std::round(vx * 100.0f) / 100.0f;
				vy = std::round(vy * 100.0f) / 100.0f;
			}
			_polygons[_activePoly].vertices.push_back(SpriteVertex(vx, vy));
			_luaDirty = true;
			_luaEdited = false;
			applyToDef();
		}
		_dragPoly = _dragVert = _dragCircle = -1;
		_dragCircleRadius = false;
		_dragging = false;
		_pressOnVertex = false;
	}

	if (hovered && ImGui::IsKeyPressed(ImGuiKey_Delete, false) && _hoverPoly >= 0 && _hoverVert >= 0) {
		SpritePolygon::SpriteVector& verts = _polygons[_hoverPoly].vertices;
		verts.erase(verts.begin() + _hoverVert);
		_luaDirty = true;
		_luaEdited = false;
		applyToDef();
	}

	for (int p = 0; p < static_cast<int>(_polygons.size()); ++p) {
		const SpritePolygon::SpriteVector& verts = _polygons[p].vertices;
		const bool active = p == _activePoly;
		const ImU32 lineCol = active ? IM_COL32(255, 230, 80, 255) : IM_COL32(180, 180, 200, 160);
		const ImU32 fillCol = active ? IM_COL32(255, 200, 40, 40) : IM_COL32(140, 160, 200, 28);
		if (verts.size() >= 3) {
			std::vector<ImVec2> pts;
			pts.reserve(verts.size());
			for (size_t i = 0; i < verts.size(); ++i)
				pts.push_back(toScreen(origin, tilePixels, verts[i].x, verts[i].y));
			if (isConvex(verts)) {
				std::vector<ImVec2> fillPts = pts;
				// Screen Y is down, so a clockwise Lua polygon (Y up) is counter-clockwise here.
				if (shoelaceSigned(verts) < 0.0f)
					std::reverse(fillPts.begin(), fillPts.end());
				drawList->AddConvexPolyFilled(&fillPts[0], static_cast<int>(fillPts.size()), fillCol);
			}
			for (size_t i = 0; i < verts.size(); ++i) {
				const ImVec2 a = pts[i];
				const ImVec2 b = pts[(i + 1) % pts.size()];
				const bool closing = i + 1 == verts.size();
				drawList->AddLine(a, b, closing ? IM_COL32(255, 230, 80, active ? 140 : 80) : lineCol,
						active ? 2.0f : 1.0f);
			}
		} else if (verts.size() == 2) {
			drawList->AddLine(toScreen(origin, tilePixels, verts[0].x, verts[0].y),
					toScreen(origin, tilePixels, verts[1].x, verts[1].y), lineCol, active ? 2.0f : 1.0f);
		}

		if (active && hovered && !_dragging && _hoverPoly < 0 && !verts.empty()) {
			const ImVec2 last = toScreen(origin, tilePixels, verts.back().x, verts.back().y);
			drawList->AddLine(last, mouse, IM_COL32(255, 80, 80, 160), 1.0f);
		}

		for (int v = 0; v < static_cast<int>(verts.size()); ++v) {
			const ImVec2 s = toScreen(origin, tilePixels, verts[v].x, verts[v].y);
			const bool hot = (_hoverPoly == p && _hoverVert == v) || (_dragPoly == p && _dragVert == v);
			const float r = hot ? 6.0f : 4.5f;
			drawList->AddCircleFilled(s, r, IM_COL32(220, 40, 40, 255));
			drawList->AddCircle(s, r, IM_COL32(255, 255, 255, hot ? 255 : 180), 12, 1.0f);
		}
	}

	drawList->PopClipRect();
	drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
			IM_COL32(70, 70, 80, 255));
}

void UIShapeEditor::drawLuaPanel ()
{
	if (_luaDirty && !_luaEdited)
		rebuildLua();

	ImGui::TextUnformatted(tr("Lua polygon / circle definition").c_str());
	if (ImGui::Button(tr("Copy to clipboard").c_str())) {
		ImGui::SetClipboardText(_luaText.c_str());
		setStatus(tr("Copied to clipboard"));
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Paste").c_str())) {
		const char* clip = ImGui::GetClipboardText();
		if (clip != nullptr && clip[0] != '\0') {
			_luaText = clip;
			_luaEdited = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(tr("Apply Lua").c_str())) {
		std::vector<SpritePolygon> parsed;
		std::vector<SpriteCircle> parsedCircles;
		std::string error;
		if (sprite_polygon_lua::fromLuaShapes(_luaText, parsed, parsedCircles, &error)) {
			if (parsed.empty())
				parsed.push_back(SpritePolygon(""));
			pushShapeUndo();
			_polygons = parsed;
			_circles = parsedCircles;
			_activePoly = 0;
			_activeCircle = _circles.empty() ? -1 : 0;
			std::snprintf(_userDataBuf, sizeof(_userDataBuf), "%s", _polygons[0].userData.c_str());
			_luaEdited = false;
			_luaDirty = true;
			applyToDef();
			setStatus(tr("Applied Lua definition"));
		} else {
			setStatus(error.empty() ? tr("Invalid Lua shape definition") : error);
		}
	}
	if (!_status.empty() && ImGui::GetTime() < _statusUntil) {
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "%s", _status.c_str());
	}

	ImGui::BeginChild("shape_lua", ImVec2(0, 0), true);
	if (ImGui::InputTextMultiline("##shape_lua_text", &_luaText, ImVec2(-FLT_MIN, -FLT_MIN),
			ImGuiInputTextFlags_AllowTabInput))
		_luaEdited = true;
	ImGui::EndChild();
}

void UIShapeEditor::draw (IFrontend* frontend, const std::string& suggestedSpriteId)
{
	if (!_visible)
		return;
	if (_spriteId.empty() && !suggestedSpriteId.empty())
		loadSprite(suggestedSpriteId);

	ImGui::SetNextWindowSize(ImVec2(980.0f, 640.0f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(tr("Sprite Shape Editor").c_str(), &_visible)) {
		ImGui::End();
		return;
	}

	drawSpritePicker();
	if (!_spriteId.empty()) {
		drawToolbar();
		ImGui::Separator();
		const float luaHeight = 200.0f;
		ImGui::BeginChild("shape_canvas_host", ImVec2(0.0f, -luaHeight - 8.0f), false,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		drawCanvas(frontend);
		ImGui::EndChild();
		drawLuaPanel();
	} else {
		ImGui::TextDisabled("%s", tr("Pick a sprite to edit its collision polygons.").c_str());
	}

	ImGui::End();
}
