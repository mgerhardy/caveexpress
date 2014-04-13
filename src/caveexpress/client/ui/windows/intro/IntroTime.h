#pragma once

#include "engine/common/Compiler.h"
#include "Intro.h"

class IntroTime: public Intro {
public:
	IntroTime (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};
