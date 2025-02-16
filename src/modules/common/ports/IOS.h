#pragma once

#include "Unix.h"

class IOS: public Unix {
public:
	IOS ();
	virtual ~IOS ();

public:
	int openURL (const std::string& url, bool newWindow) const override;
	bool isFullscreenSupported () override { return false; }
	bool supportsUserContent () const override { return false; }
	std::string getRateURL (const std::string& packageName) const override;
	virtual bool hasTouch () const override { return true; }
};
