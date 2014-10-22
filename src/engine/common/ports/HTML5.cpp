#include "HTML5.h"

extern "C" {
// find these in contrib/installer/html5/library.js
void _jsOpenURL (const char* url);
void _jsAlert (const char* reason);
void _jsBacktrace ();
}

HTML5::HTML5 () :
		Unix() {
}

HTML5::~HTML5 () {
}

std::string HTML5::getHomeDirectory ()
{
	return "";
}

std::string HTML5::getCurrentWorkingDir ()
{
	return "";
}

void HTML5::exit (const std::string& reason, int errorCode)
{
	if (errorCode != 0) {
		logError(reason);
		_jsAlert(reason.c_str());
	} else {
		logOutput(reason);
	}
	::exit(errorCode);
}

void HTML5::backtrace (const char *errorMessage)
{
	_jsBacktrace();
}

void HTML5::showAds (bool show)
{
}

int HTML5::openURL (const std::string& url) const
{
	_jsOpenURL(url.c_str());
	return 0;
}
