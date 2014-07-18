#pragma once

#include "engine/common/Compiler.h"
#include "engine/client/ui/windows/intro/Intro.h"

class IntroAttack: public Intro {
public:
	IntroAttack (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
