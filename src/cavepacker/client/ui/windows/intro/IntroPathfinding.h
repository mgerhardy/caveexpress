#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

namespace cavepacker {

class IntroPathfinding: public Intro {
public:
	explicit IntroPathfinding (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};

}
