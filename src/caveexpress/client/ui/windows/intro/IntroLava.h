#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

namespace caveexpress {

class IntroLava: public Intro {
public:
	explicit IntroLava (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};

}
