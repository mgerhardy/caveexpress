#include "UINodeServerSelector.h"
#include "ui/UI.h"
#include "common/CommandSystem.h"
#include "common/Logger.h"
#include "common/Commands.h"

#define MAP_HEADLINE tr("Map")
#define NAME_HEADLINE tr("Name")
#define PLAYERS_HEADLINE tr("Players")

UINodeServerSelector::UINodeServerSelector (IFrontend *frontend, int rows) :
		UINodeSelector<ServerEntry>(frontend, 1, rows)
{
	_headlineFont = getFont(HUGE_FONT);
	_headlineHeight = _headlineFont->getCharHeight();
	setBackgroundColor(backgroundColor);
	setSize(0.8f, 0.6f);
	setScrollingEnabled(true);
	setPageVisible(false);
	setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE);
	setId("server-selector");
	setFont(HUGE_FONT);
	setRowHeight(getFontHeight() / static_cast<float>(_frontend->getHeight()));
	_mouseWheelScrollAmount = _rowHeight * _frontend->getHeight() * 5;
	Vector4Set(colorWhite, _fontColor);
	reset();
	setRowSpacing(2);
	_entryOffsetY = _headlineHeight;
	_colWidth = _size.x;
	setAutoColsRows();
}

UINodeServerSelector::~UINodeServerSelector ()
{
}

void UINodeServerSelector::addServer (const std::string &host, const std::string& name, const std::string& mapName,
		int port, int playerCount, int maxPlayerCount)
{
	info(LOG_CLIENT, "add server: " + host);
	addData(ServerEntry(name, host, port, mapName, playerCount, maxPlayerCount));
}

bool UINodeServerSelector::onSelect (const ServerEntry& data)
{
	const std::string connect = CMD_CL_CONNECT " " + data.host + " " + string::toString(data.port);
	info(LOG_CLIENT, "connect via '" + connect + "'");
	Commands.executeCommandLine(connect);
	return true;
}

void UINodeServerSelector::render (int x, int y) const
{
	UINodeSelector<ServerEntry>::render(x, y);
	x += getRenderX();
	y += getRenderY();
	_frontend->renderFilledRect(x, y, _colWidth * _frontend->getWidth(), _headlineHeight, colorGray);
	_headlineFont->print(NAME_HEADLINE, _fontColor, x + getNameX(), y, false);
	_headlineFont->print(MAP_HEADLINE, _fontColor, x + getMapX(), y, false);
	_headlineFont->print(PLAYERS_HEADLINE, _fontColor, x + getPlayersX(), y, false);
}

int UINodeServerSelector::getNameX () const
{
	return 0;
}

int UINodeServerSelector::getMapX () const
{
	return getRenderWidth() / 2;
}

int UINodeServerSelector::getPlayersX () const
{
	return getRenderWidth() - _headlineFont->getTextWidth(PLAYERS_HEADLINE);
}

void UINodeServerSelector::renderSelectorEntry (int index, const ServerEntry& data, int x, int y, int colWidth, int rowHeight, float alpha) const
{
	Color color = { 0.6f, 0.6f, 0.6f, 0.6f };
	if (_selectedIndex == index) {
		color[0] = color[1] = color[2] = 1.0f;
		color[3] = 0.3f;
	} else if ((index % 2) == 0) {
		color[3] = 0.3f;
	}

	const int nameWidth = getMapX() - getNameX() - _rowSpacing;
	const int mapNameWidth = getPlayersX() - getMapX() - _rowSpacing;
	const std::string players = string::toString(data.playerCount) + "/" + string::toString(data.maxPlayerCount);
	const int playersWidth = _font->getTextWidth(PLAYERS_HEADLINE) - _rowSpacing;

	_frontend->renderFilledRect(x, y, colWidth, rowHeight, color);
	_font->printMax(data.name, _fontColor, x + getNameX(), y, nameWidth, false);
	_font->printMax(data.mapName, _fontColor, x + getMapX(), y, mapNameWidth, false);
	_font->printMax(players, _fontColor, x + getPlayersX(), y, playersWidth, false);
}

bool UINodeServerSelector::onPush ()
{
	reset();
	Commands.executeCommandLine(CMD_CL_PINGSERVERS);
	return true;
}
