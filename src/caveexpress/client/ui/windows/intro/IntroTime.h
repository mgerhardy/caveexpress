#pragma once

#include "engine/common/Compiler.h"
#include "engine/client/ui/windows/intro/Intro.h"

class IntroTime: public Intro {
public:
	IntroTime (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
