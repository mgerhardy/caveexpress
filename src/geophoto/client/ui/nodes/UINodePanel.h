#pragma once

#include "client/ui/nodes/UINode.h"
#include "common/Pointers.h"
#include <vector>

enum UINodePanelLayout {
	LAYOUT_NONE,
	LAYOUT_GRID_X,
	LAYOUT_GRID_Y
};

class UINodePanel: public UINode {
protected:
	typedef std::vector<UINode*> UINodeList;
	typedef UINodeList::const_iterator UINodeListConstIter;
	typedef UINodeList::iterator UINodeListIter;
	typedef UINodeList::const_reverse_iterator UINodeListConstRevIter;
	typedef UINodeList::reverse_iterator UINodeListRevIter;

	UINodeList _nodes;
	UINodePanelLayout _layout;
	float _xShift;
	float _yShift;
	float _horizontalSpacing;
	float _verticalSpacing;
	bool _noAutoSizeOnAdd;

	bool checkPanelAABB (UINode* node, int32_t x, int32_t y) const;

public:
	UINodePanel (IFrontend *frontend, float w = 0.0f, float h = 0.0f);
	~UINodePanel ();

	void setLayout (UINodePanelLayout layout);
	void add (UINode* node);

	void setSpacing (float horizontal, float vertical);

	// UINode
	float getAutoWidth () const override;
	float getAutoHeight () const override;
	bool wantFocus () const override;
	UINode* getNode (const std::string& nodeId) override;
	void addFocus (int32_t x, int32_t y) override;
	bool execute (int x, int y) override;
	bool hasMultipleFocus () override;
	void removeFocus () override;
	bool onPush () override;
	void initDrag (int32_t x, int32_t y) override;
	bool onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override;
	bool onMouseWheel (int32_t x, int32_t y) override;
	bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button) override;
	bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button) override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y) override;
	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;
	void onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override;
	void render (int x, int y) const override;
	void update (uint32_t deltaTime) override;
};

inline void UINodePanel::setSpacing (float horizontal, float vertical)
{
	_horizontalSpacing = horizontal;
	_verticalSpacing = vertical;
}
