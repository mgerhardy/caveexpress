#include "IOSObjc.h"
#include <SDL.h>
#include <UIKit/UIApplication.h>

int iosOpenURL (const std::string& url) {
	@autoreleasepool
	{
		UIApplication *app = [UIApplication sharedApplication];
		NSURL *nsurl = [NSURL URLWithString:@(url.c_str())];

		if ([app canOpenURL:nsurl]) {
			const bool success = [app openURL:nsurl];
			return success ? 0 : -1;
		}
		return -1;
	}
}
