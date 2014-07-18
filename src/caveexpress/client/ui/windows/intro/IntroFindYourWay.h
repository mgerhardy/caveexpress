#pragma once

#include "engine/common/Compiler.h"
#include "engine/client/ui/windows/intro/Intro.h"

class IntroFindYourWay: public Intro {
public:
	IntroFindYourWay (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
