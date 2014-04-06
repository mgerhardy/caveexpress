#include "Cocoa.h"
#include <SDL.h>
#include <SDL_platform.h>
#ifndef __IPHONEOS__
#import <Cocoa/Cocoa.h>
#endif
#include <sys/stat.h>

void nslogOutput (const std::string& string)
{
	NSString *format = [NSString stringWithCString:string.c_str()];
	NSLog(@"%@", format);
}

#if 1
static void uncaughtExceptionHandler (NSException *exception)
{
	NSLog(@"CRASH: %@", exception);
	NSLog(@"Stack Trace: %@", [exception callStackSymbols]);
}
#endif

void nsinit ()
{
	NSSetUncaughtExceptionHandler(&uncaughtExceptionHandler);
}

char* nsGetHomeDirectory (const char *app)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSArray *array = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    char *retval = NULL;

    if ([array count] > 0) {  // we only want the first item in the list.
        NSString *str = [array objectAtIndex:0];
        const char *base = [str UTF8String];
        if (base) {
            const size_t len = SDL_strlen(base) + SDL_strlen(app) + 3;
            retval = (char *) SDL_malloc(len);
            if (retval == NULL) {
                SDL_OutOfMemory();
            } else {
                char *ptr;
                SDL_snprintf(retval, len, "%s/%s/", base, app);
                for (ptr = retval+1; *ptr; ptr++) {
                    if (*ptr == '/') {
                        *ptr = '\0';
                        mkdir(retval, 0700);
                        *ptr = '/';
                    }
                }
                mkdir(retval, 0700);
            }
        }
    }

    [pool release];
    return retval;
}
