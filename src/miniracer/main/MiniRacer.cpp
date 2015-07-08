#include "MiniRacer.h"

namespace miniracer {

MiniRacer::MiniRacer ()
{
}

MiniRacer::~MiniRacer ()
{
}

}

static GameRegisterStatic MiniRacer("miniracer", GamePtr(new miniracer::MiniRacer()));
