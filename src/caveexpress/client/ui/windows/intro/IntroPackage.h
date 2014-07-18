#pragma once

#include "engine/common/Compiler.h"
#include "engine/client/ui/windows/intro/Intro.h"

class IntroPackage: public Intro {
public:
	IntroPackage (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
