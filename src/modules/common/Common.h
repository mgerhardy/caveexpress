#pragma once

#include "Compiler.h"
#include <memory>
#include <stdarg.h>

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__

void cp_snprintf(char* dest, size_t size, const char* fmt, ...) __attribute__((format(__printf__, 3, 4)));
inline void cp_snprintf(char* dest, size_t size, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(dest, size, fmt, ap);
	dest[size - 1] = '\0';
	va_end(ap);
}
#else
void cp_snprintf(char* dest, size_t size, const char* fmt, ...) __attribute__((format(__printf__, 3, 4)));
inline void cp_snprintf(char* dest, size_t size, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(dest, size, fmt, ap);
	va_end(ap);
}
#endif

#define lengthof(x) (sizeof(x) / sizeof(*(x)))
#define CASSERT(x) extern int ASSERT_COMPILE[((x) != 0) * 2 - 1]
