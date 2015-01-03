#include "HTML5.h"

extern "C" {
// find these in contrib/installer/html5/library.js
void _jsOpenURL (const char* url, bool newWindow);
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
	return "/user_data/";
}

std::string HTML5::getCurrentWorkingDir ()
{
	return "";
}

void HTML5::syncFiles() {
	logOutput("sync files");
	EM_ASM(
		FS.syncfs(function(error) {
			if (error) { console.log("Error while syncing", error); }
			console.log('finished syncing..');
		});
	);
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

int HTML5::openURL (const std::string& url, bool newWindow) const
{
	_jsOpenURL(url.c_str(), newWindow);
	return 0;
}
