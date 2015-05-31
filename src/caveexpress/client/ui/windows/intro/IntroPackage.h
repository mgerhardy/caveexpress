#pragma once

#include "common/Compiler.h"
#include "client/ui/windows/intro/Intro.h"

class IntroPackage: public Intro {
public:
	IntroPackage (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
