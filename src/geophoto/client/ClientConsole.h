#pragma once

#include "shared/IConsole.h"
#include "shared/IEventObserver.h"
#include "client/ui/BitmapFont.h"
#include <string>
#include <vector>
#include <inttypes.h>

class ClientConsole: public IConsole {
private:
	// console is shown
	bool _active;
	int _padding;

	BitmapFontPtr _font;
	Color _fontColor;

	typedef std::vector<std::string> ConsoleText;
	typedef ConsoleText::const_iterator ConsoleTextConstIter;
	ConsoleText _text;

	IFrontend *_frontend;

	// draw the commandline
	void commandLineDraw () const;
	void toggleConsole ();

public:
	ClientConsole ();
	virtual ~ClientConsole ();

	bool isActive () const;

	// IConsole
	void init (IFrontend *frontend) override;
	void render () override;
	void logInfo (const std::string& string) override;
	void logError (const std::string& string) override;
	void logDebug (const std::string& string) override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	void update (uint32_t deltaTime) override;
};

inline bool ClientConsole::isActive () const
{
	return _active;
}
