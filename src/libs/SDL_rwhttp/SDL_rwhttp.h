#ifndef _SDL_RWHTTP_
#define _SDL_RWHTTP_

#include <SDL.h>
#include <SDL_version.h>
#include <begin_code.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#define SDL_RWHTTP_MAJOR_VERSION   0
#define SDL_RWHTTP_MINOR_VERSION   1
#define SDL_RWHTTP_PATCHLEVEL      0

#define SDL_RWHTTP_VERSION(x) \
{ \
    (x)->major = SDL_RWHTTP_MAJOR_VERSION; \
    (x)->minor = SDL_RWHTTP_MINOR_VERSION; \
    (x)->patch = SDL_RWHTTP_PATCHLEVEL;    \
}

#define SDL_RWOPS_HTTP 404

/**
 * \brief A variable that lets you specify the user agent the requests are using.
 *
 * \note The value can not be changed after \c SDL_RWHttpInit was called.
 */
#define SDL_RWHTTP_HINT_USER_AGENT "SDL_RWHTTP_USERAGENT"

/**
 * \brief
 *
 * \note The value can not be changed after \c SDL_RWHttpInit was called.
 */
#define SDL_RWHTTP_HINT_CONNECTTIMEOUT "SDL_RWHTTP_CONNECTTIMEOUT"

/**
 * \brief
 *
 * \note The value can not be changed after \c SDL_RWHttpInit was called.
 */
#define SDL_RWHTTP_HINT_TIMEOUT "SDL_RWHTTP_TIMEOUT"

/**
 * \brief Defines the max. allowed size of a file to download.
 *
 * \note The value can not be changed after \c SDL_RWHttpInit was called.
 */
#define SDL_RWHTTP_HINT_FETCHLIMIT "SDL_RWHTTP_FETCHLIMIT"

/**
 * \brief Initializes the library. Should only be called once per application.
 * Also initializes and caches the hint values.
 *
 * \return -1 if any error occurred, 0 otherwise.
 */
extern DECLSPEC int SDL_RWHttpInit (void);

/*
 * \brief Cleanup the library.
 *
 * \return -1 if any error occurred, 0 otherwise.
 */
extern DECLSPEC int SDL_RWHttpShutdown (void);

/**
 * \name RWFrom functions
 *
 * Functions to create SDL_RWops structures from http streams.
 */
extern DECLSPEC SDL_RWops* SDL_RWFromHttpAsync (const char *uri);
extern DECLSPEC SDL_RWops* SDL_RWFromHttpSync (const char *uri);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include <close_code.h>

#endif
