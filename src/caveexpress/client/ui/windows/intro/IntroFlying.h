#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

class IntroFlying: public Intro {
public:
	IntroFlying (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
