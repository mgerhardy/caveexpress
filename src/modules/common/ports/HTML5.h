#pragma once

#include "Unix.h"
#include <emscripten.h>

class HTML5 : public Unix {
public:
	HTML5 ();
	virtual ~HTML5 ();

	std::string getHomeDirectory () override;
	std::string getCurrentWorkingDir () override;
	void exit (const std::string& reason, int errorCode) override;
	void syncFiles() override;
	int openURL (const std::string& url, bool newWindow) const override;
	void backtrace (const char *errorMessage) override;
	bool supportsUserContent () const override { return false; }
};
