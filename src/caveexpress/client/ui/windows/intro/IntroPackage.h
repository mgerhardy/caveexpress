#pragma once

#include "engine/common/Compiler.h"
#include "Intro.h"

class IntroPackage: public Intro {
public:
	IntroPackage (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
