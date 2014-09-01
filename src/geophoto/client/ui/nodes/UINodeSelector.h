#pragma once

#include "UINode.h"
#include "client/ui/BitmapFont.h"
#include "shared/Logger.h"

#include <vector>

template<class T>
class UINodeSelector: public UINode {
protected:
	typedef std::vector<T> SelectorEntries;
	typedef typename SelectorEntries::const_iterator SelectorEntryConstIter;
	typedef typename SelectorEntries::iterator SelectorEntryIter;
	SelectorEntries _entries;

	// current render positions
	mutable int _renderX;
	mutable int _renderY;

	bool _scrollingEnabled;

	int _scrolling;
	int _offset;

	int _rowSpacing;
	int _colSpacing;

	// rows and cols to use for rendering the entries
	int _cols;
	int _rows;

	int _selectedIndex;

	T* _selection;

	BitmapFontPtr _font;
	Color _fontColor;

	bool _pageVisible;

	int _colWidth;
	int _rowHeight;

	bool select ();

	virtual void renderSelectorEntry (int index, const T& data, int x, int y, int colWidth, int rowHeight, float alpha) const
	{
		const TexturePtr t = getIcon(data);
		if (t)
			renderImage(t, x, y, colWidth, rowHeight, alpha);
	}

public:
	UINodeSelector (IFrontend *frontend, int cols, int rows, int colWidth = 128, int rowHeight = 128) :
			UINode(frontend), _renderX(0), _renderY(0), _scrollingEnabled(true), _scrolling(0), _offset(0), _rowSpacing(0), _colSpacing(0), _cols(
					cols), _rows(rows), _selectedIndex(-1), _selection(nullptr), _pageVisible(false), _colWidth(colWidth), _rowHeight(rowHeight)
	{
		_font = getFont();
		Vector4Set(colorWhite, _fontColor);
		_renderBorder = true;
		const int spacing = 10;
		const float padding = 0.01f;
		const float width = 2.0f * padding + ((cols * colWidth) + (rows - 1) * spacing) / static_cast<float>(_frontend->getWidth());
		const float height = 2.0f * padding + ((rows * rowHeight) + (cols - 1) * spacing) / static_cast<float>(_frontend->getHeight());
		setSize(width, height);
		setColSpacing(spacing);
		setRowSpacing(spacing);
		setPadding(padding);
	}

	virtual ~UINodeSelector ()
	{
	}

	virtual void reset ()
	{
		_entries.clear();
		_scrolling = 0;
		_selection = nullptr;
		_offset = 0;
	}

	// if the width and height is set, we can determine how many entries fit into the selector
	inline void setAutoColsRows ()
	{
		_rows = getRenderHeight() / _rowHeight;
		_cols = getRenderWidth() / _colWidth;
	}

	inline T* getSelection () const
	{
		return _selection;
	}

	inline void setScrollingEnabled (bool scrollingEnabled)
	{
		_scrollingEnabled = scrollingEnabled;
	}

	inline int getCols () const
	{
		return _cols;
	}

	inline int getRows () const
	{
		return _rows;
	}

	void setFont (const std::string& font)
	{
		_font = getFont(font);
		_rowHeight = _font->getCharHeight();
		setAutoColsRows();
		autoSize();
	}

	void addData (const T &data)
	{
		_entries.push_back(data);
	}

	// called when the given entry was selected
	virtual bool onSelect (const T& data)
	{
		return true;
	}

	// get the icon for rendering the given entry
	virtual TexturePtr getIcon (const T& data) const
	{
		return TexturePtr();
	}

	virtual std::string getText (const T& data) const
	{
		return "";
	}

	void setColSpacing (int colSpacing);
	void setRowSpacing (int rowSpacing);

	bool hasMoreEntries () const
	{
		const int amount = _entries.size();
		if (amount == 0)
			return false;
		const int amountPerPage = _rows * _cols;
		if (amountPerPage == 0)
			return false;
		const int pages = amount / amountPerPage + ((amount % amountPerPage > 0) ? 1 : 0);
		if (pages <= 1)
			return false;

		const int currentPage = _offset / amountPerPage;
		return currentPage < pages;
	}

	bool hasLessEntries () const
	{
		return _offset > 0;
	}

	// UINode
	bool wantFocus () const override
	{
		return true;
	}

	bool onMouseLeftRelease (int32_t x, int32_t y) override
	{
		const bool ret = select();
		UINode::onMouseLeftRelease(x, y);
		return ret;
	}

	bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y) override
	{
		addFocus(x, y);
		const bool ret = select();
		UINode::onFingerRelease(finger, x, y);
		return ret;
	}

	void onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override
	{
		if (!_scrollingEnabled)
			return;
		scroll(dy > 0, 10);
	}

	bool onMouseWheel (int32_t x, int32_t y) override
	{
		if (!_scrollingEnabled)
			return false;
		scroll(y > 0, 10);
		return true;
	}

	/**
	 * @brief Scroll the node entries
	 * @param[in] up If @c true we scroll upwards, if @c false, downwards
	 * @param[in] amount The amount of pixels to scroll. This is capped to [0,-maxX]
	 */
	void scroll (bool up, int amount)
	{
		if (!_scrollingEnabled)
			return;
		const int rowHeight = _rowHeight + _rowSpacing;
		const int entries = _entries.size();
		const int rows = entries / _cols + (entries % _cols != 0 ? 1 : 0);
		const int maxRows = getRenderHeight(false) / rowHeight;
		if (maxRows > rows)
			return;

		const int maxY = (rows - maxRows) * rowHeight;

		if (up)
			_scrolling += amount;
		else
			_scrolling -= amount;

		if (_scrolling < -maxY)
			_scrolling = -maxY;
		else if (_scrolling > 0)
			_scrolling = 0;
	}

	void render (int x, int y) const override
	{
		UINode::render(x, y);

		_renderX = 0;
		_renderY = 0;

		if (_pageVisible) {
			const int amount = _entries.size();
			const int amountPerPage = _rows * _cols;
			const int pages = amount / amountPerPage + ((amount % amountPerPage > 0) ? 1 : 0);
			if (pages > 1) {
				const int currentPage = _offset / amountPerPage;
				const int pageGap = 5;
				const int pageWidth = 20;
				const int pagesWidth = pages * (pageWidth + pageGap);
				const int pageX = getRenderCenterX() - pagesWidth / 2;
				const int _y = y + getRenderY(false) - pageWidth / 2;
				for (int i = 0; i < pages; ++i) {
					const int _x = x + pageX + i * (pageWidth + pageGap);
					if (i == currentPage)
						_frontend->renderFilledRect(_x, _y, pageWidth, pageWidth, colorWhite);
					else
						_frontend->renderFilledRect(_x, _y, pageWidth, pageWidth, colorGray);
				}
			}
		}

		enableScissor(x, y);
		int index = 0;
		for (SelectorEntryConstIter i = _entries.begin(); i != _entries.end(); ++i, ++index) {
			if (index < _offset)
				continue;
			const int _x = x + getRenderX() + _renderX;
			const int _y = y + _scrolling + getRenderY() + _renderY;

			float alpha = 1.0f;
			if (_selectedIndex == index) {
				alpha = 0.5f;
			}

			renderSelectorEntry(index, *i, _x, _y, _colWidth, _rowHeight, alpha);

			if (_font) {
				const std::string& text = getText(*i);
				_font->print(text, _fontColor, _x, _y);
			}

			_renderX += _colWidth + _colSpacing;
			if (_renderX >= getRenderWidth()) {
				_renderX = 0;
				_renderY += _rowHeight + _rowSpacing;
			}
			if (_renderY + _scrolling >= getRenderHeight()) {
				break;
			}
		}

		disableScissor();
	}

	bool hasMultipleFocus () override
	{
		return _selectedIndex != _entries.size() - 1;
	}

	bool onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override
	{
		UINode::onMouseMotion(x, y, relX, relY);
		addFocus(x, y);
		return true;
	}

	bool execute (int x, int y) override
	{
		if (x == -1 && y == -1) {
			select();
			return true;
		}
		return UINode::execute(x, y);
	}

	void removeFocus () override
	{
		UINode::removeFocus();
		_selectedIndex = -1;
	}

	void addFocus (int32_t x, int32_t y) override
	{
		UINode::addFocus(x, y);
		if (_entries.empty()) {
			_selectedIndex = -1;
			return;
		}
		if (x == -1 && y == -1) {
			++_selectedIndex;
			_selectedIndex %= _entries.size();
			return;
		}
		const int relX = x - getRenderX();
		const int relY = y - getRenderY() - _scrolling;
		const int xEntry = relX / (_colWidth + _colSpacing);
		const int yEntry = (relY + (_rowHeight + _rowSpacing)) / (_rowHeight + _rowSpacing);

		_selectedIndex = (yEntry - 1) * _cols + xEntry + _offset;
	}

	void offset (bool add = true, int value = -1)
	{
		if (value <= 0)
			value = _cols * _rows;

		if (add) {
			if (_offset + value < 0)
				return;

			if (_offset + value >= _entries.size())
				return;

			_offset += value;
		} else {
			if (_offset - value < 0)
				return;

			_offset -= value;
		}
	}

	float getAutoWidth () const override
	{
		const float w = (_colWidth * _cols + (_cols - 1) * _colSpacing) / static_cast<float>(_frontend->getWidth()) + 2.0f * getPadding();
		return w;
	}

	float getAutoHeight () const override
	{
		const float h = (_rowHeight * _rows + (_rows - 1) * _rowSpacing) / static_cast<float>(_frontend->getHeight()) + 2.0f * getPadding();
		return h;
	}

	void setPageVisible (bool pageVisible)
	{
		_pageVisible = pageVisible;
	}
};

template<class T>
class UINodeBackgroundSelector: public UINodeSelector<T> {
public:
	UINodeBackgroundSelector (IFrontend *frontend, int cols, int rows, int colWidth = 128, int rowHeight = 128) :
			UINodeSelector<T>(frontend, cols, rows, colWidth, rowHeight)
	{
		this->setBackgroundColor(backgroundColor);
		this->setSize(0.8f, 0.6f);
		this->setScrollingEnabled(false);
		this->setAutoColsRows();
		this->autoSize();
		this->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
	}
};

template<class T>
inline void UINodeSelector<T>::setColSpacing (int colSpacing)
{
	_colSpacing = colSpacing;
}

template<class T>
inline void UINodeSelector<T>::setRowSpacing (int rowSpacing)
{
	_rowSpacing = rowSpacing;
}

template<class T>
inline bool UINodeSelector<T>::select ()
{
	if (_selectedIndex == -1)
		return false;

	if (_selectedIndex >= _entries.size())
		return false;

	_selection = &_entries[_selectedIndex];
	return onSelect(*_selection);
}
