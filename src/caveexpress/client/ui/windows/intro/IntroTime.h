#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

namespace caveexpress {

class IntroTime: public Intro {
public:
	explicit IntroTime (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};

}
