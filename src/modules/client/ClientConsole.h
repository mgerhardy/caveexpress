#pragma once

#include "common/IConsole.h"
#include "common/IEventObserver.h"
#include "ui/BitmapFont.h"
#include <string>
#include <vector>
#include <inttypes.h>

class ClientConsole: public IConsole {
private:
	int _padding;

	std::string _font;
	Color _fontColor;

	typedef std::vector<std::string> ConsoleText;
	typedef ConsoleText::const_iterator ConsoleTextConstIter;
	typedef ConsoleText::const_reverse_iterator ConsoleTextConstRevIter;
	ConsoleText _text;

	IFrontend *_frontend;

	// draw the commandline
	void commandLineDraw (const BitmapFontPtr& font) const;
	void toggleConsole ();

public:
	ClientConsole ();
	virtual ~ClientConsole ();

	// IConsole
	void init (IFrontend *frontend) override;
	void render () override;
	void logInfo (const std::string& string) override;
	void logError (const std::string& string) override;
	void logDebug (const std::string& string) override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	void update (uint32_t deltaTime) override;
};
