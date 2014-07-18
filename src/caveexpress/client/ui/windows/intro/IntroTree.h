#pragma once

#include "engine/common/Compiler.h"
#include "engine/client/ui/windows/intro/Intro.h"

class IntroTree: public Intro {
public:
	IntroTree (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
