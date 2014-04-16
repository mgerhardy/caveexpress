#pragma once

#include "engine/common/Compiler.h"
#include "Intro.h"

class IntroFlying: public Intro {
public:
	IntroFlying (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
