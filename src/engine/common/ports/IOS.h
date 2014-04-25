#pragma once

#include "engine/common/ports/Unix.h"

class IOS: public Unix {
public:
	IOS ();
	virtual ~IOS ();

public:
	void logError (const std::string& error) const override;
	void logOutput (const std::string& string) const override;
	void showAds (bool show) override;
	int openURL (const std::string& url) const override;
	std::string getHomeDirectory () override;
};

