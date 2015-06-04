#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

class IntroFindYourWay: public Intro {
public:
	explicit IntroFindYourWay (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
