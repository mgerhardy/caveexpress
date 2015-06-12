#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

namespace cavepacker {

class IntroGame: public Intro {
public:
	explicit IntroGame (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};

}
