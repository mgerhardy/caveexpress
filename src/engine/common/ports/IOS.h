#pragma once

#include "engine/common/ports/Unix.h"

class IOS: public Unix {
public:
	IOS ();
	virtual ~IOS ();

public:
	void showAds (bool show) override;
	int openURL (const std::string& url) const override;
	std::string getHomeDirectory () override;
};

