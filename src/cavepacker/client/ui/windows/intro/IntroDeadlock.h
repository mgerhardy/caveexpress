#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

namespace cavepacker {

class IntroDeadlock: public Intro {
public:
	explicit IntroDeadlock (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};

}
