#pragma once

#include "Unix.h"

class HTML5 : public Unix {
public:
	HTML5 ();
	virtual ~HTML5 ();

	std::string getHomeDirectory () override;
	std::string getCurrentWorkingDir () override;
	void exit (const std::string& reason, int errorCode) override;
	void showAds (bool show) override;
	int openURL (const std::string& url) const override;
	void backtrace (const char *errorMessage) override;
};

