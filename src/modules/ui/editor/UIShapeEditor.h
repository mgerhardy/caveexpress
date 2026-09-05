#pragma once

#include "common/SpriteDefinition.h"
#include "imgui.h"
#include <string>
#include <vector>

class IFrontend;

class UIShapeEditor {
	bool _visible = false;
	std::string _spriteId;
	std::vector<SpritePolygon> _polygons;
	std::vector<SpriteCircle> _circles;
	int _activePoly = 0;
	int _activeCircle = -1;
	int _dragPoly = -1;
	int _dragVert = -1;
	int _dragCircle = -1;
	bool _dragCircleRadius = false;
	bool _dragging = false;
	std::vector<SpritePolygon> _undoPolygons;
	std::vector<SpriteCircle> _undoCircles;
	bool _hasShapeUndo = false;
	ImVec2 _pressMouse;
	bool _pressOnVertex = false;
	char _filter[128] = {};
	char _userDataBuf[64] = {};
	float _zoom = 4.0f;
	float _panX = 0.0f;
	float _panY = 0.0f;
	std::string _luaText;
	bool _luaDirty = true;
	bool _luaEdited = false;
	std::string _status;
	double _statusUntil = 0.0;
	int _hoverPoly = -1;
	int _hoverVert = -1;

	SpriteDefPtr currentDef () const;
	void loadSprite (const std::string& id);
	void applyToDef ();
	void rebuildLua ();
	void setStatus (const std::string& text);
	void ensureActivePolygon ();
	bool hitVertex (const ImVec2& origin, float tilePixels, const ImVec2& mouse, int& outPoly, int& outVert) const;
	int hitCircle (const ImVec2& origin, float tilePixels, const ImVec2& mouse, bool& onRim) const;
	void pushShapeUndo ();
	bool writeSpritesLua ();
	ImVec2 toScreen (const ImVec2& origin, float tilePixels, float vx, float vy) const;
	void toVertex (const ImVec2& origin, float tilePixels, const ImVec2& screen, float& vx, float& vy) const;
	void drawSpritePicker ();
	void drawToolbar ();
	void drawCanvas (IFrontend* frontend);
	void drawLuaPanel ();

public:
	bool isVisible () const { return _visible; }
	void setVisible (bool visible) { _visible = visible; }
	void toggle () { _visible = !_visible; }
	void open (const std::string& suggestedSpriteId);
	void draw (IFrontend* frontend, const std::string& suggestedSpriteId);
};
