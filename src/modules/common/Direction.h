#pragma once

#include <stdint.h>

#define DIRECTION_UP (1 << 0)
#define DIRECTION_DOWN (1 << 1)
#define DIRECTION_LEFT (1 << 2)
#define DIRECTION_RIGHT (1 << 3)

#define DIRECTION_HORIZONTAL (DIRECTION_LEFT | DIRECTION_RIGHT)
#define DIRECTION_VERTICAL (DIRECTION_UP | DIRECTION_DOWN)

typedef uint8_t Direction;
