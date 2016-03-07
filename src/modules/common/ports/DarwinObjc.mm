#include "DarwinObjc.h"
#include <SDL.h>
#include <SDL_platform.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#include <sys/stat.h>
#include <unistd.h>

static void uncaughtExceptionHandler (NSException *exception)
{
	NSLog(@"CRASH: %@", exception);
	NSLog(@"Stack Trace: %@", [exception callStackSymbols]);
}

void darwinInit ()
{
	NSSetUncaughtExceptionHandler(&uncaughtExceptionHandler);
	char *basePath = SDL_GetBasePath();
	if (basePath != nullptr) {
		chdir(basePath);
		SDL_free(basePath);
	}
}

void darwinRequestUserAttention(bool critical)
{
	@autoreleasepool
	{
		if (critical) {
			[NSApp requestUserAttention:NSCriticalRequest];
			return;
		}
		[NSApp requestUserAttention:NSInformationalRequest];
	}
}
