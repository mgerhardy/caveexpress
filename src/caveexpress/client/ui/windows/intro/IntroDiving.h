#pragma once

#include "common/Compiler.h"
#include "ui/windows/intro/Intro.h"

namespace caveexpress {

class IntroDiving: public Intro {
public:
	explicit IntroDiving (IFrontend* frontend);
protected:
	void addIntroNodes(UINode* parent) override;
};

}
