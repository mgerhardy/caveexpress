#include "UINodePanel.h"

UINodePanel::UINodePanel (IFrontend *frontend, float w, float h) :
		UINode(frontend), _layout(LAYOUT_GRID_X), _xShift(0.0f), _yShift(0.0f), _horizontalSpacing(0.0f), _verticalSpacing(0.0f)
{
	setSize(w, h);
	_noAutoSizeOnAdd = w >= EPSILON && h >= EPSILON;
}

UINodePanel::~UINodePanel ()
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		delete *i;
	}
	_nodes.clear();
}

void UINodePanel::update (uint32_t deltaTime)
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		(*i)->update(deltaTime);
	}
}

void UINodePanel::render (int x, int y) const
{
	UINode::render(x, y);
	x += getRenderX();
	y += getRenderY();
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		(*i)->render(x, y);
	}
}

void UINodePanel::initDrag (int32_t x, int32_t y)
{
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		if (checkPanelAABB(node, x, y)) {
			node->initDrag(x - panelX, y - panelY);
		}
	}
}

bool UINodePanel::onPush ()
{
	bool push = UINode::onPush();
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		push |= (*i)->onPush();
	}
	return push;
}

void UINodePanel::setLayout (UINodePanelLayout layout)
{
	_layout = layout;
}

bool UINodePanel::wantFocus () const
{
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		if (node->wantFocus())
			return true;;
	}
	return false;
}

void UINodePanel::addFocus (int32_t x, int32_t y)
{
	UINode::addFocus(x, y);
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		if (x == -1 && y == -1) {
			if (!node->hasFocus())
				continue;

			if (!node->hasMultipleFocus()) {
				node->removeFocus();
				for (;;) {
					++i;
					if (i == _nodes.end())
						i = _nodes.begin();
					node = *i;
					if (!node->wantFocus())
						continue;
					break;
				}
			}
			node->addFocus();
			return;
		}
		if (checkPanelAABB(node, x, y)) {
			if (!node->hasFocus())
				node->addFocus(x - panelX, y - panelY);
		} else if (node->hasFocus()) {
			node->removeFocus();
		}
	}

	if (x == -1 && y == -1) {
		for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
			UINode* nodePtr = *i;
			if (!nodePtr->wantFocus())
				continue;
			nodePtr->addFocus();
			break;
		}
	}
}

bool UINodePanel::execute (int x, int y)
{
	if (x != -1 && y != -1)
		return UINode::execute(x, y);

	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->execute();
		return true;
	}

	return false;
}

bool UINodePanel::hasMultipleFocus ()
{
	if (_nodes.empty())
		return false;
	return !_nodes.back()->hasFocus();
}

void UINodePanel::removeFocus ()
{
	UINode::removeFocus();
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		if (!node->hasFocus())
			continue;
		node->removeFocus();
	}
}

bool UINodePanel::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		if (!checkPanelAABB(node, x, y))
			continue;
		if (node->onMouseButtonPress(x - panelX, y - panelY, button))
			return true;
	}
	return UINode::onMouseButtonPress(x, y, button);
}

bool UINodePanel::onMouseWheel (int32_t x, int32_t y)
{
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		if (!node->hasFocus())
			continue;
		if (node->onMouseWheel(x, y))
			return true;
	}
	return UINode::onMouseWheel(x, y);
}

bool UINodePanel::onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	const bool val = UINode::onMouseMotion(x, y, relX, relY);
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		const bool nodeHasFocus = node->hasFocus();
		if (!node->isVisible()) {
			if (nodeHasFocus)
				node->removeFocus();
			continue;
		}
		if (checkPanelAABB(node, x, y)) {
			if (!nodeHasFocus)
				node->addFocus(x - panelX, y - panelY);
			node->onMouseMotion(x - panelX, y - panelY, relX, relY);
		} else if (nodeHasFocus) {
			node->removeFocus();
		}
	}
	return val;
}

bool UINodePanel::checkPanelAABB (UINode* node, int32_t x, int32_t y) const
{
	const float _x = x / static_cast<float>(_frontend->getWidth());
	const float _y = y / static_cast<float>(_frontend->getHeight());
	return checkAABB(_x, _y, _pos.x + _padding + node->_pos.x, _pos.y + _padding + node->_pos.y, node->_size.x, node->_size.y);
}

bool UINodePanel::onFingerRelease (int64_t finger, uint16_t x, uint16_t y)
{
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		if (!checkPanelAABB(node, x, y))
			continue;
		if (node->onFingerRelease(finger, x - panelX, y - panelY))
			return true;
	}
	return UINode::onFingerRelease(finger, x, y);
}

bool UINodePanel::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		if (!checkPanelAABB(node, x, y))
			continue;
		if (node->onFingerPress(finger, x - panelX, y - panelY))
			return true;
	}
	return UINode::onFingerPress(finger, x, y);
}

void UINodePanel::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		if (!checkPanelAABB(node, x, y))
			continue;
		node->onFingerMotion(finger, x - panelX, y - panelY, dx, dy);
	}
	UINode::onFingerMotion(finger, x, y, dx, dy);
}

bool UINodePanel::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		if (!checkPanelAABB(node, x, y))
			continue;
		if (node->onMouseButtonRelease(x - panelX, y - panelY, button))
			return true;
	}
	return UINode::onMouseButtonRelease(x, y, button);
}

bool UINodePanel::onKeyPress (int32_t key, int16_t modifier)
{
	if (UINode::onKeyPress(key, modifier))
		return true;
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* node = *i;
		if (node->onKeyPress(key, modifier))
			return true;
	}
	return false;
}

void UINodePanel::add (UINode* node)
{
	_nodes.push_back(node);
	if (!_noAutoSizeOnAdd && node->isVisible())
		autoSize();
	switch (_layout) {
	case LAYOUT_GRID_X:
		if (node->getX() + node->getWidth() + _xShift > getWidth()) {
			_xShift = 0.0f;
			_yShift += node->getHeight() + _verticalSpacing;
		}
		node->_pos.x += _xShift;
		_xShift += node->getWidth() + _horizontalSpacing;
		node->_pos.y += _yShift;
		break;
	case LAYOUT_GRID_Y: {
		const float y = node->getBottom() + _yShift;
		const float panelHeight = getHeight();
		if (y > panelHeight) {
			_yShift = 0.0f;
			_xShift += node->getWidth() + _horizontalSpacing;
		}
		node->_pos.y += _yShift;
		_yShift += node->getHeight() + _verticalSpacing;
		node->_pos.x += _xShift;
		break;
	}
	default:
		break;
	}
	updateAlignment();
}

float UINodePanel::getAutoWidth () const
{
	float w = 0.0f;
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		switch (_layout) {
		case LAYOUT_GRID_X:
			w += (*i)->getWidth();
			break;
		case LAYOUT_GRID_Y:
			w = std::max(_size.x, (*i)->getWidth() + 2.0f * _padding);
			break;
		default:
			break;
		}
	}
	if (_layout == LAYOUT_GRID_X)
		w += (_nodes.size() + 1) * _padding + (_nodes.size() - 1) * _horizontalSpacing;
	return w;
}

float UINodePanel::getAutoHeight () const
{
	float h = 0.0f;
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		switch (_layout) {
		case LAYOUT_GRID_X:
			h = std::max(h, (*i)->getHeight() + 2.0f * _padding);
			break;
		case LAYOUT_GRID_Y:
			h += (*i)->getHeight();
			break;
		default:
			break;
		}
	}
	if (_layout == LAYOUT_GRID_Y)
		h += (_nodes.size() + 1) * _padding + (_nodes.size() - 1) * _verticalSpacing;
	return h;
}

UINode* UINodePanel::getNode (const std::string& nodeId)
{
	UINode *node = UINode::getNode(nodeId);
	if (node)
		return node;

	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		node = (*i)->getNode(nodeId);
		if (node)
			return node;
	}

	return node;
}
