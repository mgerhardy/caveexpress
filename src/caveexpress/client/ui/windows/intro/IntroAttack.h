#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

namespace caveexpress {

class IntroAttack: public Intro {
public:
	explicit IntroAttack (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};

}
