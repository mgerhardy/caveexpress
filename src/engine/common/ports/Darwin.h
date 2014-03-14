#pragma once

#include "engine/common/ports/Unix.h"

class Darwin: public Unix {
public:
	Darwin ();
	virtual ~Darwin ();

public:
	void logError (const std::string& error) const override;
	void logOutput (const std::string& string) const override;
	int openURL (const std::string& url) const override;
	std::string getHomeDirectory () override;
};
