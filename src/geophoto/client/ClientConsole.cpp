#include "ClientConsole.h"
#include "client/ui/UI.h"
#include "shared/CommandSystem.h"
#include "shared/IFrontend.h"
#include "shared/Logger.h"
#include "shared/Version.h"
#include "shared/ConfigManager.h"
#include "shared/EventHandler.h"
#include "common/System.h"
#include <SDL.h>
#include <SDL_platform.h>
#include <iostream>

ClientConsole::ClientConsole () :
		IConsole(), _active(false), _padding(2), _font(BitmapFontPtr()), _frontend(nullptr)
{
	Commands.registerCommand(CMD_CL_TOGGLECONSOLE, bind(ClientConsole, toggleConsole));
	Logger::get().addConsole(this);
}

ClientConsole::~ClientConsole ()
{
	Logger::get().removeConsole(this);
}

void ClientConsole::init (IFrontend *frontend)
{
	_frontend = frontend;
	_font = UI::get().getFont();
	Vector4Set(colorBlack, _fontColor);
}

void ClientConsole::render ()
{
	if (!_active)
		return;

	_frame++;
	if ((_frame % 10) == 0)
		_cursorBlink ^= true;

	const Color color = { 1.0, 1.0, 1.0, 0.6 };
	_frontend->renderFilledRect(0, 0, 0, 0, color);

	if (!_text.empty()) {
		int y = (_text.size() - 1) * _font->getCharHeight();
		const int newY = _frontend->getHeight() - _padding * _font->getCharHeight();
		if (y > newY)
			y = newY;

		for (ConsoleTextConstIter i = _text.end() - 1; i != _text.begin(); --i) {
			_font->print(*i, _fontColor, _padding, y);
			y -= _font->getCharHeight();
			if (y < 0)
				break;
		}
	}

	commandLineDraw();
}

void ClientConsole::toggleConsole ()
{
	_active ^= true;
	if (_active)
		SDL_StartTextInput();
	else
		SDL_StopTextInput();
}

void ClientConsole::logInfo (const std::string& string)
{
	System.logOutput(string);
	_text.push_back(string);
}

void ClientConsole::logError (const std::string& string)
{
	System.logError(string);
	_text.push_back(string);
}

void ClientConsole::logDebug (const std::string& string)
{
	if (!Config.isDebug()) {
		return;
	}
	logInfo(string);
}

void ClientConsole::update (uint32_t deltaTime)
{
}

bool ClientConsole::onKeyPress (int32_t key, int16_t modifier)
{
	switch (key) {
	case SDLK_TAB:
		if (modifier & KMOD_SHIFT) {
			toggleConsole();
			return true;
		}
	}

	if (!_active)
		return false;

	switch (key) {
	case SDLK_ESCAPE:
		toggleConsole();
		break;
	case SDLK_RETURN:
		executeCommandLine();
		break;
	case SDLK_BACKSPACE:
		cursorDelete();
		break;
	case SDLK_DELETE:
		cursorDelete(false);
		break;
	case SDLK_INSERT:
		_overwrite ^= true;
		break;
	case SDLK_LEFT:
		cursorLeft();
		break;
	case SDLK_RIGHT:
		cursorRight();
		break;
	case SDLK_UP:
		cursorUp();
		break;
	case SDLK_DOWN:
		cursorDown();
		break;
	case SDLK_TAB:
		if (!modifier) {
			autoComplete();
			break;
		}
		return false;
	default:
		return false;
	}

	return true;
}

void ClientConsole::commandLineDraw () const
{
	std::string s = "]" + _commandLine;
	if (_cursorBlink)
		s += "_";
	_font->print(s, _fontColor, _padding, _frontend->getHeight() - _font->getCharHeight());
}
