#pragma once

#include "engine/common/Compiler.h"
#include "Intro.h"

class Intro1: public Intro {
public:
	Intro1 (IFrontend* frontend);
protected:
	void addIntroNodes() override;
};
