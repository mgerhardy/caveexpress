#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

class IntroGeyser: public Intro {
public:
	explicit IntroGeyser (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
