#pragma once

#include "engine/common/Compiler.h"
#include "engine/client/ui/windows/intro/Intro.h"

class IntroGame: public Intro {
public:
	IntroGame (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
