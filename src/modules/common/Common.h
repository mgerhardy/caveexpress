#pragma once

#include "Compiler.h"
#include <memory>

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define lengthof(x) (sizeof(x) / sizeof(*(x)))
#define CASSERT(x) extern int ASSERT_COMPILE[((x) != 0) * 2 - 1]
