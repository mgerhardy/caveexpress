#pragma once

#include <stdint.h>

#define BIT_SET(n,b) (((n) |= 1 << (b)))
#define BIT_CLEAR(n,b) ((n) &= ~(1 << (b)))
#define BIT_CHECK(n,b) ((n) & (1 << (b)))
#define BIT_SETV(n,b,v) (((v) > 0) ? ((n) |= 1 << (b)) : ((n) &= ~(1 << (b))))

typedef uint8_t BitSet8;
typedef int16_t BitSet16;
typedef int32_t BitSet32;
typedef int64_t BitSet64;
