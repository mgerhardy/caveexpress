#pragma once

#include "common/Compiler.h"
#include "client/ui/windows/intro/Intro.h"

class IntroTree: public Intro {
public:
	IntroTree (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
