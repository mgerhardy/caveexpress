#pragma once

#include "Compiler.h"
#include "Pointers.h"

#define lengthof(x) (sizeof(x) / sizeof(*(x)))
#define CASSERT(x) extern int ASSERT_COMPILE[((x) != 0) * 2 - 1]
