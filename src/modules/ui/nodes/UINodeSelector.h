#pragma once

#include "UINode.h"
#include "ui/BitmapFont.h"
#include "common/Log.h"
#include "common/ConfigManager.h"

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

	int _mouseWheelScrollAmount;
	// scrolling pixels
	int _scrolling;
	// amount of entries to skip while showing the selector
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

	// show page indicator
	bool _pageVisible;

	float _colWidth;
	float _rowHeight;

	int _cursorX;
	int _cursorY;

	// initial offset for the first entry in the selector
	int _entryOffsetX;
	int _entryOffsetY;

	bool select ();

	virtual void renderSelectorEntry (int index, const T& data, int x, int y, int colWidth, int rowHeight, float alpha) const
	{
		const TexturePtr t = getIcon(data);
		if (t)
			renderImage(t, x, y, colWidth, rowHeight, alpha);
	}

public:
	UINodeSelector (IFrontend *frontend, int cols, int rows, float colWidth = 0.2f, float rowHeight = 0.2f) :
			UINode(frontend), _renderX(0), _renderY(0), _scrollingEnabled(true), _mouseWheelScrollAmount(10), _scrolling(0), _offset(0), _rowSpacing(0), _colSpacing(0), _cols(
					cols), _rows(rows), _selectedIndex(-1), _selection(nullptr), _pageVisible(false), _colWidth(colWidth), _rowHeight(rowHeight), _cursorX(0), _cursorY(0),
					_entryOffsetX(0), _entryOffsetY(0)
	{
		_font = getFont();
		Vector4Set(colorWhite, _fontColor);
		autoSize();
		if (isSmallScreen()) {
			_colWidth *= 1.5f;
			_rowHeight *= 1.5f;
			setAutoColsRows();
			autoSize();
		}
	}

	virtual ~UINodeSelector ()
	{
	}

	inline void setDimensions(int cols, int rows)
	{
		_cols = cols;
		_rows = rows;
	}

	virtual void onAdd () override
	{
		UINode::onAdd();
		autoSize();
		updateAlignment();
	}

	void defaults ()
	{
		setPageVisible(true);
		setStandardPadding();
		setBorder(true);
		setRowSpacing(4);
		setColSpacing(4);
	}

	// uses a reference texture to define the rows and cols that are shown in this selector
	void setColsRowsFromTexture (const std::string& refTexture)
	{
		const TexturePtr& refPicForSelector = loadTexture(refTexture);
		if (!refPicForSelector->isValid())
			return;
		const int rw = getRenderWidth(false);
		const int rh = getRenderHeight(false);
		const int cols = std::max(1, rw / refPicForSelector->getWidth());
		const int rows = std::max(1, rh / refPicForSelector->getHeight());
		setLayoutData(cols, rows, refPicForSelector->getWidth() / static_cast<float>(_frontend->getWidth()),
				refPicForSelector->getHeight() / static_cast<float>(_frontend->getHeight()));
	}

	inline void setLayoutData (int cols, int rows, float colWidth, float rowHeight)
	{
		_cols = cols;
		_rows = rows;
		_colWidth = colWidth;
		_rowHeight = rowHeight;
		autoSize();
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
		_rows = std::max(1, (int)((_size.y - 2.0f * _padding) / (_rowHeight + _rowSpacing / static_cast<float>(_frontend->getHeight()))));
		_cols = std::max(1, (int)((_size.x - 2.0f * _padding) / (_colWidth + _colSpacing / static_cast<float>(_frontend->getWidth()))));
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

	inline void setFont (const std::string& font)
	{
		_font = getFont(font);
	}

	inline void setRowHeight (float rowHeight)
	{
		_rowHeight = rowHeight;
	}

	inline int getFontHeight () const
	{
		return _font->getCharHeight();
	}

	inline void addData (const T &data)
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

	inline bool hasLessEntries () const
	{
		return _offset > 0;
	}

	bool isActive () const override
	{
		return isVisible();
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

	bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy) override
	{
		if (!_scrollingEnabled)
			return false;
		scroll(dy > 0, 10);
		return true;
	}

	bool onMouseWheel (int32_t x, int32_t y) override
	{
		if (!_scrollingEnabled) {
			// TODO: only offset one row
			offset(true);
			selectEntry(_offset);
			return false;
		}
		scroll(y > 0, _mouseWheelScrollAmount);
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
		const int rowHeight = _rowHeight * _frontend->getHeight() + _rowSpacing;
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

	virtual void render (int x, int y) const override
	{
		UINode::render(x, y);

		_renderX = _entryOffsetX;
		_renderY = _entryOffsetY;

		if (_pageVisible) {
			const int amount = _entries.size();
			const int amountPerPage = _rows * _cols;
			if (amountPerPage != 0) {
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
		}

		enableScissor(x, y);
		int index = 0;
		const int colWidth = _colWidth * _frontend->getWidth();
		const int rowHeight = _rowHeight * _frontend->getHeight();
		for (SelectorEntryConstIter i = _entries.begin(); i != _entries.end(); ++i, ++index) {
			if (index < _offset)
				continue;
			const int _x = x + getRenderX() + _renderX;
			const int _y = y + _scrolling + getRenderY() + _renderY;

			float alpha = 1.0f;
			if (_selectedIndex == index) {
				alpha = 0.5f;
			}

			renderSelectorEntry(index, *i, _x, _y, colWidth, rowHeight, alpha);

			if (_font) {
				const std::string& text = getText(*i);
				_font->print(text, _fontColor, _x, _y);
			}

			_renderX += colWidth + _colSpacing;
			if (_renderX >= getRenderWidth()) {
				_renderX = 0;
				_renderY += rowHeight + _rowSpacing;
			}
			if (_renderY + _scrolling >= getRenderHeight()) {
				break;
			}
		}

		disableScissor();
	}

	virtual bool onKeyPress (int32_t key, int16_t modifier) override
	{
		if (!UINode::onKeyPress(key, modifier)) {
			if (key == SDLK_PAGEUP) {
				offset(false);
				selectEntry(_offset);
				return true;
			} else if (key == SDLK_PAGEDOWN) {
				offset(true);
				selectEntry(_offset);
				return true;
			}
		}
		return false;
	}

	void onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override
	{
		UINode::onMouseMotion(x, y, relX, relY);
		addFocus(x, y);
		_cursorX = x;
		_cursorY = y;
	}

	void renderDebug (int x, int y, int textY) const override
	{
		UINode::renderDebug(x, y, textY);
		const BitmapFontPtr& font = getFont(MEDIUM_FONT);
		font->print(String::format("x: %i, y: %i, index: %i", _cursorX, _cursorY, _selectedIndex), colorWhite, x, textY);
	}

	bool runFocusNode () override
	{
		return select();
	}

	void removeFocus () override
	{
		UINode::removeFocus();
		_selectedIndex = -1;
	}

	bool nextFocus (bool cursordown) override
	{
		if (_entries.empty())
			return false;
		if (cursordown)
			_selectedIndex += _cols;
		else
			++_selectedIndex;
		if (_selectedIndex - _offset >= _rows * _cols) {
			offset(true);
			_selectedIndex = _offset;
		}
		Log::info(LOG_UI, "selected index: %i, offset: %i", _selectedIndex, _offset);
		_selectedIndex %= (int) _entries.size();
		if (_selectedIndex == 0) {
			return UINode::nextFocus(cursordown);
		}
		return true;
	}

	bool prevFocus (bool cursorup) override
	{
		if (_entries.empty())
			return false;
		if (cursorup)
			_selectedIndex -= _cols;
		else
			--_selectedIndex;
		if (_selectedIndex < _offset) {
			offset(false);
			_selectedIndex = (_rows * _cols) - 1;
		}
		if (_selectedIndex < 0) {
			_selectedIndex = _entries.size() - 1;
			_offset = _selectedIndex % (_rows * _cols);
			return UINode::prevFocus(cursorup);
		}
		return true;
	}

	void addFocus (int32_t x, int32_t y) override
	{
		UINode::addFocus(x, y);
		if (_entries.empty()) {
			_selectedIndex = -1;
			return;
		}
		if (x == 0 && y == 0) {
			_selectedIndex = 0;
			return;
		}
		const int relX = x - getRenderX() + _entryOffsetX;
		const int relY = y - getRenderY() - _scrolling - _entryOffsetY;
		const int colWidth = _colWidth * _frontend->getWidth() + _colSpacing;
		const int rowHeight = _rowHeight * _frontend->getHeight() + _rowSpacing;
		const int xEntry = relX / colWidth;
		const int yEntry = (relY + rowHeight) / rowHeight;

		_selectedIndex = (yEntry - 1) * _cols + xEntry + _offset;
	}

	/**
	 * @brief Select the given entry
	 *
	 * @param[in] index The index (starting at 0) of the item in the selector that should get activated
	 */
	void selectEntry (int index)
	{
		const int amountPerPage = _rows * _cols;
		if (index <= 0) {
			_offset = 0;
			_selectedIndex = 0;
		} else if (index < static_cast<int>(_entries.size())) {
			const int neededPage = (index + 1) / amountPerPage;
			_offset = neededPage * amountPerPage;
			Log::debug(LOG_UI, "Scroll to page %i (index was: %i, amountPerPage is: %i)", neededPage, index, amountPerPage);
			_selectedIndex = index;
		}
	}

	void offset (bool increase = true, int value = -1)
	{
		if (value <= 0)
			value = _cols * _rows;

		if (increase) {
			if (_offset + value < 0)
				return;

			if (_offset + value >= static_cast<int>(_entries.size()))
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
		const float w = _colWidth * _cols + (_cols - 1) * (_colSpacing / static_cast<float>(_frontend->getWidth())) + 2.0f * getPadding();
		return std::min(1.0f, w);
	}

	float getAutoHeight () const override
	{
		const float h = _rowHeight * _rows + (_rows - 1) * (_rowSpacing / static_cast<float>(_frontend->getHeight())) + 2.0f * getPadding();
		return std::min(1.0f, h);
	}

	void setPageVisible (bool pageVisible)
	{
		_pageVisible = pageVisible;
	}
};

template<class T>
class UINodeBackgroundSelector: public UINodeSelector<T> {
public:
	UINodeBackgroundSelector (IFrontend *frontend, int cols, int rows) :
			UINodeSelector<T>(frontend, cols, rows, 0.8f / static_cast<float>(cols), 0.6f / static_cast<float>(rows))
	{
		this->setBackgroundColor(backgroundColor);
		this->setSize(0.8f, 0.6f);
		this->setScrollingEnabled(false);
		this->setAutoColsRows();
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

	if (_selectedIndex >= static_cast<int>(_entries.size()))
		return false;

	_selection = &_entries[_selectedIndex];
	return onSelect(*_selection);
}
