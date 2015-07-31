#pragma once

#if !defined(__GNUC__)
#define  __attribute__(x)
#endif

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#define DEPRECATED __attribute__((__deprecated__))
#else
#define DEPRECATED
#endif

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#define DEPRECATED_FOR(f) __attribute__((deprecated("Use " #f " instead")))
#else
#define DEPRECATED_FOR(f) DEPRECATED
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpointer-sign"
#endif

#ifndef __INTEL_COMPILER
# if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#  define GCC_DIAG_STR(s) #s
#  define GCC_DIAG_JOINSTR(x,y) GCC_DIAG_STR(x ## y)
#  define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#  define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#  if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#   define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#   define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
#  else
#   define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#   define GCC_DIAG_ON(x)  GCC_DIAG_PRAGMA(warning GCC_DIAG_JOINSTR(-W,x))
#  endif
# else
#  define GCC_DIAG_OFF(x)
#  define GCC_DIAG_ON(x)
# endif
#else
# define GCC_DIAG_OFF(x)
# define GCC_DIAG_ON(x)
#endif
