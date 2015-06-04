#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

class IntroTree: public Intro {
public:
	explicit IntroTree (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
