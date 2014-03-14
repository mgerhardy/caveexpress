#include "UINodeMapStringSelector.h"
#include "engine/common/MapManager.h"

UINodeMapStringSelector::UINodeMapStringSelector (IFrontend *frontend, const MapManager &mapManager, int rows) :
		UINodeSelector<std::string>(frontend, 1, rows), _mapManager(mapManager)
{
	_rows = rows;
	setId("map-string-selector");
	setBackgroundColor(colorWhite);
	setBorder(true);
	setVisible(false);
	setBorderColor(colorBlack);
	setFont(MEDIUM_FONT);
	setRowHeight(getFontHeight() / static_cast<float>(_frontend->getHeight()));
	Vector4Set(colorBlack, _fontColor);
	reset();
	setRowSpacing(2);
	autoSize();
	_colWidth = _size.x;
	setAutoColsRows();
	_mouseWheelScrollAmount = _rowHeight * _frontend->getHeight() * 5;
}

UINodeMapStringSelector::~UINodeMapStringSelector ()
{
}

float UINodeMapStringSelector::getAutoHeight () const
{
	return std::min(_rows, static_cast<int>(_entries.size())) * _font->getCharHeight() / static_cast<float>(_frontend->getHeight());
}

float UINodeMapStringSelector::getAutoWidth () const
{
	float w = 0.0f;
	for (SelectorEntryConstIter i = _entries.begin(); i != _entries.end(); ++i) {
		w = std::max(w, _font->getTextWidth(*i) / static_cast<float>(_frontend->getWidth()));
	}
	return w;
}

void UINodeMapStringSelector::renderSelectorEntry (int index, const std::string& data, int x, int y, int colWidth, int rowHeight, float alpha) const
{
	if (_selectedIndex == index) {
		_frontend->renderFilledRect(x, y, colWidth, rowHeight, colorGray);
	}
}

std::string UINodeMapStringSelector::getText (const std::string& data) const
{
	return data;
}

void UINodeMapStringSelector::reset ()
{
	UINodeSelector<std::string>::reset();
	const MapManager::Maps &maps = _mapManager.getMaps();
	for (MapManager::Maps::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		addData(i->first);
	}
}
