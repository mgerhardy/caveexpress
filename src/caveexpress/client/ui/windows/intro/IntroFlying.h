#pragma once

#include "engine/common/Compiler.h"
#include "engine/client/ui/windows/intro/Intro.h"

class IntroFlying: public Intro {
public:
	IntroFlying (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
