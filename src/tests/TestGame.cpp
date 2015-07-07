#include "game/GameRegistry.h"

class TestGame: public IGame {
};

static GameRegisterStatic TESTGAME("tests", GamePtr(new TestGame()));
