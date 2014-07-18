#pragma once

#include "engine/common/Compiler.h"
#include "engine/client/ui/windows/intro/Intro.h"

class IntroGeyser: public Intro {
public:
	IntroGeyser (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
